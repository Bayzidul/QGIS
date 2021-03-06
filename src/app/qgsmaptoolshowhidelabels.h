/***************************************************************************
                          qgsmaptoolshowhidelabels.h
                          --------------------
    begin                : 2012-08-12
    copyright            : (C) 2012 by Larry Shaffer
    email                : larrys at dakotacarto dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSHOWHIDELABELS_H
#define QGSMAPTOOLSHOWHIDELABELS_H

#include "qgsmaptoollabel.h"
#include "qgsfeature.h"
#include "qgis_app.h"


//! A map tool for showing or hiding a feature's label
class APP_EXPORT QgsMapToolShowHideLabels : public QgsMapToolLabel
{
    Q_OBJECT

  public:
    QgsMapToolShowHideLabels( QgsMapCanvas *canvas );
    ~QgsMapToolShowHideLabels();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse press event
    virtual void canvasPressEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e ) override;

  protected:

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;

    //! Stores actual select rect
    QRect mSelectRect;

    //! Stores selection marquee
    QgsRubberBand* mRubberBand = nullptr;

  private:
    //! Select valid labels to pin or unpin
    void showHideLabels( QMouseEvent * e );

    //! Return features intersecting rubberband
    bool selectedFeatures( QgsVectorLayer* vlayer,
                           QgsFeatureIds& selectedFeatIds );

    //! Return label features intersecting rubberband
    bool selectedLabelFeatures( QgsVectorLayer* vlayer,
                                QList<QgsLabelPosition> &listPos );

    //! Show label or diagram with feature ID
    bool showHide( QgsVectorLayer *vl, const bool show );
};

#endif // QGSMAPTOOLSHOWHIDELABELS_H
