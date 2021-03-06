/***************************************************************************
    qgsrubberband.cpp - Rubberband widget for drawing multilines and polygons
     --------------------------------------
    Date                 : 07-Jan-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrubberband.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include <QPainter>

/*!
  \class QgsRubberBand
  \brief The QgsRubberBand class provides a transparent overlay widget
  for tracking the mouse while drawing polylines or polygons.
*/
QgsRubberBand::QgsRubberBand( QgsMapCanvas* mapCanvas, QgsWkbTypes::GeometryType geometryType )
    : QgsMapCanvasItem( mapCanvas )
    , mIconSize( 5 )
    , mIconType( ICON_CIRCLE )
    , mGeometryType( geometryType )
    , mTranslationOffsetX( 0.0 )
    , mTranslationOffsetY( 0.0 )
{
  reset( geometryType );
  QColor color( Qt::lightGray );
  color.setAlpha( 63 );
  setColor( color );
  setWidth( 1 );
  setLineStyle( Qt::SolidLine );
  setBrushStyle( Qt::SolidPattern );
}

QgsRubberBand::QgsRubberBand()
    : QgsMapCanvasItem( nullptr )
    , mIconSize( 5 )
    , mIconType( ICON_CIRCLE )
    , mGeometryType( QgsWkbTypes::PolygonGeometry )
    , mTranslationOffsetX( 0.0 )
    , mTranslationOffsetY( 0.0 )
{
}

/*!
  Set the stroke and fill color.
  */
void QgsRubberBand::setColor( const QColor & color )
{
  setStrokeColor( color );
  setFillColor( color );
}

/*!
  Set the fill color.
  */
void QgsRubberBand::setFillColor( const QColor & color )
{
  QColor fillColor( color.red(), color.green(), color.blue(), color.alpha() );
  mBrush.setColor( fillColor );
}

/*!
  Set the stroke
  */
void QgsRubberBand::setStrokeColor( const QColor & color )
{
  QColor penColor( color.red(), color.green(), color.blue(), color.alpha() );
  mPen.setColor( penColor );
}


/*!
  Set the stroke width.
  */
void QgsRubberBand::setWidth( int width )
{
  mPen.setWidth( width );
}

void QgsRubberBand::setIcon( IconType icon )
{
  mIconType = icon;
}

void QgsRubberBand::setIconSize( int iconSize )
{
  mIconSize = iconSize;
}

void QgsRubberBand::setLineStyle( Qt::PenStyle penStyle )
{
  mPen.setStyle( penStyle );
}

void QgsRubberBand::setBrushStyle( Qt::BrushStyle brushStyle )
{
  mBrush.setStyle( brushStyle );
}

/*!
  Remove all points from the shape being created.
  */
void QgsRubberBand::reset( QgsWkbTypes::GeometryType geometryType )
{
  mPoints.clear();
  mGeometryType = geometryType;
  updateRect();
  update();
}

/*!
  Add a point to the shape being created.
  */
void QgsRubberBand::addPoint( const QgsPoint & p, bool doUpdate /* = true */, int geometryIndex )
{
  if ( geometryIndex < 0 )
  {
    geometryIndex = mPoints.size() - 1;
  }

  if ( geometryIndex < 0 || geometryIndex > mPoints.size() )
  {
    return;
  }

  if ( geometryIndex == mPoints.size() )
  {
    mPoints.push_back( QList<QgsPoint>() << p );
  }

  if ( mPoints.at( geometryIndex ).size() == 2 &&
       mPoints.at( geometryIndex ).at( 0 ) == mPoints.at( geometryIndex ).at( 1 ) )
  {
    mPoints[geometryIndex].last() = p;
  }
  else
  {
    mPoints[geometryIndex] << p;
  }


  if ( doUpdate )
  {
    setVisible( true );
    updateRect();
    update();
  }
}

void QgsRubberBand::closePoints( bool doUpdate, int geometryIndex )
{
  if ( geometryIndex < 0 || geometryIndex >= mPoints.size() )
  {
    return;
  }

  if ( mPoints.at( geometryIndex ).at( 0 ) != mPoints.at( geometryIndex ).at( mPoints.at( geometryIndex ).size() - 1 ) )
  {
    mPoints[geometryIndex] << mPoints.at( geometryIndex ).at( 0 );
  }

  if ( doUpdate )
  {
    setVisible( true );
    updateRect();
    update();
  }
}


void QgsRubberBand::removePoint( int index, bool doUpdate/* = true*/, int geometryIndex/* = 0*/ )
{

  if ( mPoints.size() < geometryIndex + 1 )
  {
    return;
  }


  if ( !mPoints[geometryIndex].isEmpty() )
  {
    // negative index removes from end, e.g., -1 removes last one
    if ( index < 0 )
    {
      index = mPoints.at( geometryIndex ).size() + index;
    }
    mPoints[geometryIndex].removeAt( index );
  }

  if ( doUpdate )
  {
    updateRect();
    update();
  }
}

void QgsRubberBand::removeLastPoint( int geometryIndex, bool doUpdate/* = true*/ )
{
  removePoint( -1, doUpdate, geometryIndex );
}

/*!
  Update the line between the last added point and the mouse position.
  */
void QgsRubberBand::movePoint( const QgsPoint & p, int geometryIndex )
{
  if ( mPoints.size() < geometryIndex + 1 )
  {
    return;
  }

  if ( mPoints.at( geometryIndex ).size() < 1 )
  {
    return;
  }

  mPoints[geometryIndex].last() = p;

  updateRect();
  update();
}

void QgsRubberBand::movePoint( int index, const QgsPoint& p, int geometryIndex )
{
  if ( mPoints.size() < geometryIndex + 1 )
  {
    return;
  }

  if ( mPoints.at( geometryIndex ).size() < index )
  {
    return;
  }

  mPoints[geometryIndex][index] = p;

  updateRect();
  update();
}

void QgsRubberBand::setToGeometry( const QgsGeometry& geom, QgsVectorLayer* layer )
{
  if ( geom.isNull() )
  {
    reset( mGeometryType );
    return;
  }

  reset( geom.type() );
  addGeometry( geom, layer );
}

void QgsRubberBand::addGeometry( const QgsGeometry& geom, QgsVectorLayer* layer )
{
  if ( geom.isNull() )
  {
    return;
  }

  //maprender object of canvas
  const QgsMapSettings& ms = mMapCanvas->mapSettings();

  int idx = mPoints.size();

  switch ( geom.wkbType() )
  {

    case QgsWkbTypes::Point:
    case QgsWkbTypes::Point25D:
    {
      QgsPoint pt;
      if ( layer )
      {
        pt = ms.layerToMapCoordinates( layer, geom.asPoint() );
      }
      else
      {
        pt = geom.asPoint();
      }
      addPoint( pt, false, idx );
      removeLastPoint( idx, false );
    }
    break;

    case QgsWkbTypes::MultiPoint:
    case QgsWkbTypes::MultiPoint25D:
    {
      QgsMultiPoint mpt = geom.asMultiPoint();
      for ( int i = 0; i < mpt.size(); ++i, ++idx )
      {
        QgsPoint pt = mpt[i];
        if ( layer )
        {
          addPoint( ms.layerToMapCoordinates( layer, pt ), false, idx );
          removeLastPoint( idx, false );
        }
        else
        {
          addPoint( pt, false, idx );
          removeLastPoint( idx, false );
        }
      }
    }
    break;

    case QgsWkbTypes::LineString:
    case QgsWkbTypes::LineString25D:
    {
      QgsPolyline line = geom.asPolyline();
      for ( int i = 0; i < line.count(); i++ )
      {
        if ( layer )
        {
          addPoint( ms.layerToMapCoordinates( layer, line[i] ), false, idx );
        }
        else
        {
          addPoint( line[i], false, idx );
        }
      }
    }
    break;

    case QgsWkbTypes::MultiLineString:
    case QgsWkbTypes::MultiLineString25D:
    {

      QgsMultiPolyline mline = geom.asMultiPolyline();
      for ( int i = 0; i < mline.size(); ++i, ++idx )
      {
        QgsPolyline line = mline[i];

        if ( line.isEmpty() )
        {
          --idx;
        }

        for ( int j = 0; j < line.size(); ++j )
        {
          if ( layer )
          {
            addPoint( ms.layerToMapCoordinates( layer, line[j] ), false, idx );
          }
          else
          {
            addPoint( line[j], false, idx );
          }
        }
      }
    }
    break;

    case QgsWkbTypes::Polygon:
    case QgsWkbTypes::Polygon25D:
    {
      QgsPolygon poly = geom.asPolygon();
      QgsPolyline line = poly[0];
      for ( int i = 0; i < line.count(); i++ )
      {
        if ( layer )
        {
          addPoint( ms.layerToMapCoordinates( layer, line[i] ), false, idx );
        }
        else
        {
          addPoint( line[i], false, idx );
        }
      }
    }
    break;

    case QgsWkbTypes::MultiPolygon:
    case QgsWkbTypes::MultiPolygon25D:
    {

      QgsMultiPolygon multipoly = geom.asMultiPolygon();
      for ( int i = 0; i < multipoly.size(); ++i, ++idx )
      {
        QgsPolygon poly = multipoly[i];
        QgsPolyline line = poly[0];
        for ( int j = 0; j < line.count(); ++j )
        {
          if ( layer )
          {
            addPoint( ms.layerToMapCoordinates( layer, line[j] ), false, idx );
          }
          else
          {
            addPoint( line[j], false, idx );
          }
        }
      }
    }
    break;

    case QgsWkbTypes::Unknown:
    default:
      return;
  }

  setVisible( true );
  updateRect();
  update();
}

void QgsRubberBand::setToCanvasRectangle( QRect rect )
{
  if ( !mMapCanvas )
  {
    return;
  }

  const QgsMapToPixel* transform = mMapCanvas->getCoordinateTransform();
  QgsPoint ll = transform->toMapCoordinates( rect.left(), rect.bottom() );
  QgsPoint lr = transform->toMapCoordinates( rect.right(), rect.bottom() );
  QgsPoint ul = transform->toMapCoordinates( rect.left(), rect.top() );
  QgsPoint ur = transform->toMapCoordinates( rect.right(), rect.top() );

  reset( QgsWkbTypes::PolygonGeometry );
  addPoint( ll, false );
  addPoint( lr, false );
  addPoint( ur, false );
  addPoint( ul, true );
}

/*!
  Draw the shape in response to an update event.
  */
void QgsRubberBand::paint( QPainter* p )
{
  if ( !mPoints.isEmpty() )
  {
    p->setBrush( mBrush );
    p->setPen( mPen );

    Q_FOREACH ( const QList<QgsPoint>& line, mPoints )
    {
      QVector<QPointF> pts;
      Q_FOREACH ( const QgsPoint& pt, line )
      {
        const QPointF cur = toCanvasCoordinates( QgsPoint( pt.x() + mTranslationOffsetX, pt.y() + mTranslationOffsetY ) ) - pos();
        if ( pts.empty() || std::abs( pts.back().x() - cur.x() ) > 1 ||  std::abs( pts.back().y() - cur.y() ) > 1 )
          pts.append( cur );
      }

      switch ( mGeometryType )
      {
        case QgsWkbTypes::PolygonGeometry:
        {
          p->drawPolygon( pts );
        }
        break;

        case QgsWkbTypes::PointGeometry:
        {
          Q_FOREACH ( QPointF pt, pts )
          {
            double x = pt.x();
            double y = pt.y();

            qreal s = ( mIconSize - 1 ) / 2.0;

            switch ( mIconType )
            {
              case ICON_NONE:
                break;

              case ICON_CROSS:
                p->drawLine( QLineF( x - s, y, x + s, y ) );
                p->drawLine( QLineF( x, y - s, x, y + s ) );
                break;

              case ICON_X:
                p->drawLine( QLineF( x - s, y - s, x + s, y + s ) );
                p->drawLine( QLineF( x - s, y + s, x + s, y - s ) );
                break;

              case ICON_BOX:
                p->drawLine( QLineF( x - s, y - s, x + s, y - s ) );
                p->drawLine( QLineF( x + s, y - s, x + s, y + s ) );
                p->drawLine( QLineF( x + s, y + s, x - s, y + s ) );
                p->drawLine( QLineF( x - s, y + s, x - s, y - s ) );
                break;

              case ICON_FULL_BOX:
                p->drawRect( x - s, y - s, mIconSize, mIconSize );
                break;

              case ICON_CIRCLE:
                p->drawEllipse( x - s, y - s, mIconSize, mIconSize );
                break;
            }
          }
        }
        break;

        case QgsWkbTypes::LineGeometry:
        default:
        {
          p->drawPolyline( pts );
        }
        break;
      }
    }
  }
}

void QgsRubberBand::updateRect()
{
  if ( mPoints.empty() )
  {
    setRect( QgsRectangle() );
    setVisible( false );
    return;
  }

  const QgsMapToPixel& m2p = *( mMapCanvas->getCoordinateTransform() );

  qreal res = m2p.mapUnitsPerPixel();
  qreal w = (( mIconSize - 1 ) / 2 + mPen.width() ) / res;

  QgsRectangle r;
  for ( int i = 0; i < mPoints.size(); ++i )
  {
    QList<QgsPoint>::const_iterator it = mPoints.at( i ).constBegin(),
                                         itE = mPoints.at( i ).constEnd();
    for ( ; it != itE; ++it )
    {
      QgsPoint p( it->x() + mTranslationOffsetX, it->y() + mTranslationOffsetY );
      p = m2p.transform( p );
      QgsRectangle rect( p.x() - w, p.y() - w, p.x() + w, p.y() + w );

      if ( r.isEmpty() )
      {
        // Get rectangle of the first point
        r = rect;
      }
      else
      {
        r.combineExtentWith( rect );
      }
    }
  }

  // This is an hack to pass QgsMapCanvasItem::setRect what it
  // expects (encoding of position and size of the item)
  QgsPoint topLeft = m2p.toMapPoint( r.xMinimum(), r.yMinimum() );
  QgsRectangle rect( topLeft.x(), topLeft.y(), topLeft.x() + r.width()*res, topLeft.y() - r.height()*res );

  setRect( rect );
}

void QgsRubberBand::updatePosition()
{
  // re-compute rectangle
  // See http://hub.qgis.org/issues/12392
  // NOTE: could be optimized by saving map-extent
  //       of rubberband and simply re-projecting
  //       that to device-rectangle on "updatePosition"
  updateRect();
}

void QgsRubberBand::setTranslationOffset( double dx, double dy )
{
  mTranslationOffsetX = dx;
  mTranslationOffsetY = dy;
  updateRect();
}

int QgsRubberBand::size() const
{
  return mPoints.size();
}

int QgsRubberBand::partSize( int geometryIndex ) const
{
  if ( geometryIndex < 0 || geometryIndex >= mPoints.size() ) return 0;
  return mPoints[geometryIndex].size();
}

int QgsRubberBand::numberOfVertices() const
{
  int count = 0;
  QList<QList<QgsPoint> >::const_iterator it = mPoints.constBegin();
  for ( ; it != mPoints.constEnd(); ++it )
  {
    QList<QgsPoint>::const_iterator iter = it->constBegin();
    for ( ; iter != it->constEnd(); ++iter )
    {
      ++count;
    }
  }
  return count;
}

const QgsPoint *QgsRubberBand::getPoint( int i, int j ) const
{
  if ( i < mPoints.size() && j < mPoints[i].size() )
    return &mPoints[i][j];
  else
    return nullptr;
}

QgsGeometry QgsRubberBand::asGeometry() const
{
  QgsGeometry geom;

  switch ( mGeometryType )
  {
    case QgsWkbTypes::PolygonGeometry:
    {
      QgsPolygon polygon;
      QList< QList<QgsPoint> >::const_iterator it = mPoints.constBegin();
      for ( ; it != mPoints.constEnd(); ++it )
      {
        polygon.append( getPolyline( *it ) );
      }
      geom = QgsGeometry::fromPolygon( polygon );
      break;
    }

    case QgsWkbTypes::PointGeometry:
    {
      QgsMultiPoint multiPoint;

      QList< QList<QgsPoint> >::const_iterator it = mPoints.constBegin();
      for ( ; it != mPoints.constEnd(); ++it )
      {
        multiPoint += getPolyline( *it );
      }
      geom = QgsGeometry::fromMultiPoint( multiPoint );
      break;
    }

    case QgsWkbTypes::LineGeometry:
    default:
    {
      if ( !mPoints.isEmpty() )
      {
        if ( mPoints.size() > 1 )
        {
          QgsMultiPolyline multiPolyline;
          QList< QList<QgsPoint> >::const_iterator it = mPoints.constBegin();
          for ( ; it != mPoints.constEnd(); ++it )
          {
            multiPolyline.append( getPolyline( *it ) );
          }
          geom = QgsGeometry::fromMultiPolyline( multiPolyline );
        }
        else
        {
          geom = QgsGeometry::fromPolyline( getPolyline( mPoints.at( 0 ) ) );
        }
      }
      break;
    }
  }
  return geom;
}

QgsPolyline QgsRubberBand::getPolyline( const QList<QgsPoint> & points )
{
  QgsPolyline polyline;
  QList<QgsPoint>::const_iterator iter = points.constBegin();
  for ( ; iter != points.constEnd(); ++iter )
  {
    polyline.append( *iter );
  }
  return polyline;
}
