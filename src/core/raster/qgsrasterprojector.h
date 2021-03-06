/***************************************************************************
    qgsrasterprojector.h - Raster projector
     --------------------------------------
    Date                 : Jan 16, 2011
    Copyright            : (C) 2005 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* This code takes ideas from WarpBuilder in Geotools.
 * Thank to Ing. Andrea Aime, Ing. Simone Giannecchini and GeoSolutions S.A.S.
 * See : http://geo-solutions.blogspot.com/2011/01/developers-corner-improving.html
 */

#ifndef QGSRASTERPROJECTOR_H
#define QGSRASTERPROJECTOR_H

#include "qgis_core.h"
#include <QVector>
#include <QList>

#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrasterinterface.h"

#include <cmath>

class QgsPoint;
class QgsCoordinateTransform;

/** \ingroup core
 * \brief QgsRasterProjector implements approximate projection support for
 * it calculates grid of points in source CRS for target CRS + extent
 * which are used to calculate affine transformation matrices.
 * \class QgsRasterProjector
 */
class CORE_EXPORT QgsRasterProjector : public QgsRasterInterface
{
  public:

    /** Precision defines if each pixel is reprojected or approximate reprojection based
     *  on an approximation matrix of reprojected points is used.
     */
    enum Precision
    {
      Approximate = 0, //!< Approximate (default), fast but possibly inaccurate
      Exact = 1,   //!< Exact, precise but slow
    };

    QgsRasterProjector();

    QgsRasterProjector *clone() const override;

    int bandCount() const override;

    Qgis::DataType dataType( int bandNo ) const override;

    //! \brief set source and destination CRS
    void setCrs( const QgsCoordinateReferenceSystem & srcCRS, const QgsCoordinateReferenceSystem & destCRS,
                 int srcDatumTransform = -1, int destDatumTransform = -1 );

    //! \brief Get source CRS
    QgsCoordinateReferenceSystem sourceCrs() const { return mSrcCRS; }

    //! \brief Get destination CRS
    QgsCoordinateReferenceSystem destinationCrs() const { return mDestCRS; }

    Precision precision() const { return mPrecision; }
    void setPrecision( Precision precision ) { mPrecision = precision; }
    // Translated precision mode, for use in ComboBox etc.
    static QString precisionLabel( Precision precision );

    QgsRasterBlock *block( int bandNo, const QgsRectangle & extent, int width, int height, QgsRasterBlockFeedback* feedback = nullptr ) override;

    //! Calculate destination extent and size from source extent and size
    bool destExtentSize( const QgsRectangle& srcExtent, int srcXSize, int srcYSize,
                         QgsRectangle& destExtent, int& destXSize, int& destYSize );

    //! Calculate destination extent and size from source extent and size
    static bool extentSize( const QgsCoordinateTransform& ct,
                            const QgsRectangle& srcExtent, int srcXSize, int srcYSize,
                            QgsRectangle& destExtent, int& destXSize, int& destYSize );

  private:

    //! Source CRS
    QgsCoordinateReferenceSystem mSrcCRS;

    //! Destination CRS
    QgsCoordinateReferenceSystem mDestCRS;

    //! Source datum transformation id (or -1 if none)
    int mSrcDatumTransform;

    //! Destination datum transformation id (or -1 if none)
    int mDestDatumTransform;

    //! Requested precision
    Precision mPrecision;

};

/// @cond PRIVATE

/**
 * Internal class for reprojection of rasters - either exact or approximate.
 * QgsRasterProjector creates it and then keeps calling srcRowCol() to get source pixel position
 * for every destination pixel position.
 */
class ProjectorData
{
  public:
    //! Initialize reprojector and calculate matrix
    ProjectorData( const QgsRectangle &extent, int width, int height, QgsRasterInterface *input, const QgsCoordinateTransform &inverseCt, QgsRasterProjector::Precision precision );
    ~ProjectorData();

    ProjectorData( const ProjectorData& other ) = delete;
    ProjectorData& operator=( const ProjectorData& other ) = delete;

    /** \brief Get source row and column indexes for current source extent and resolution
        If source pixel is outside source extent srcRow and srcCol are left unchanged.
        @return true if inside source
     */
    bool srcRowCol( int destRow, int destCol, int *srcRow, int *srcCol );

    QgsRectangle srcExtent() const { return mSrcExtent; }
    int srcRows() const { return mSrcRows; }
    int srcCols() const { return mSrcCols; }

  private:

    //! \brief get destination point for _current_ destination position
    void destPointOnCPMatrix( int row, int col, double *theX, double *theY );

    //! \brief Get matrix upper left row/col indexes for destination row/col
    int matrixRow( int destRow );
    int matrixCol( int destCol );

    //! \brief Get precise source row and column indexes for current source extent and resolution
    inline bool preciseSrcRowCol( int destRow, int destCol, int *srcRow, int *srcCol );

    //! \brief Get approximate source row and column indexes for current source extent and resolution
    inline bool approximateSrcRowCol( int destRow, int destCol, int *srcRow, int *srcCol );

    //! \brief insert rows to matrix
    void insertRows( const QgsCoordinateTransform& ct );

    //! \brief insert columns to matrix
    void insertCols( const QgsCoordinateTransform& ct );

    //! Calculate single control point in current matrix
    void calcCP( int row, int col, const QgsCoordinateTransform& ct );

    //! \brief calculate matrix row
    bool calcRow( int row, const QgsCoordinateTransform& ct );

    //! \brief calculate matrix column
    bool calcCol( int col, const QgsCoordinateTransform& ct );

    //! \brief calculate source extent
    void calcSrcExtent();

    //! \brief calculate minimum source width and height
    void calcSrcRowsCols();

    /** \brief check error along columns
      * returns true if within threshold */
    bool checkCols( const QgsCoordinateTransform& ct );

    /** \brief check error along rows
      * returns true if within threshold */
    bool checkRows( const QgsCoordinateTransform& ct );

    //! Calculate array of src helper points
    void calcHelper( int matrixRow, QgsPoint *points );

    //! Calc / switch helper
    void nextHelper();

    //! Get mCPMatrix as string
    QString cpToString();

    /** Use approximation (requested precision is Approximate and it is possible to calculate
     *  an approximation matrix with a sufficient precision) */
    bool mApproximate;

    //! Transformation from destination CRS to source CRS
    QgsCoordinateTransform* mInverseCt = nullptr;

    //! Destination extent
    QgsRectangle mDestExtent;

    //! Source extent
    QgsRectangle mSrcExtent;

    //! Source raster extent
    QgsRectangle mExtent;

    //! Number of destination rows
    int mDestRows;

    //! Number of destination columns
    int mDestCols;

    //! Destination x resolution
    double mDestXRes;

    //! Destination y resolution
    double mDestYRes;

    //! Number of source rows
    int mSrcRows;

    //! Number of source columns
    int mSrcCols;

    //! Source x resolution
    double mSrcXRes;

    //! Source y resolution
    double mSrcYRes;

    //! Number of destination rows per matrix row
    double mDestRowsPerMatrixRow;

    //! Number of destination cols per matrix col
    double mDestColsPerMatrixCol;

    //! Grid of source control points
    QList< QList<QgsPoint> > mCPMatrix;

    //! Grid of source control points transformation possible indicator
    /* Same size as mCPMatrix */
    QList< QList<bool> > mCPLegalMatrix;

    //! Array of source points for each destination column on top of current CPMatrix grid row
    /* Warning: using QList is slow on access */
    QgsPoint *pHelperTop = nullptr;

    //! Array of source points for each destination column on bottom of current CPMatrix grid row
    /* Warning: using QList is slow on access */
    QgsPoint *pHelperBottom = nullptr;

    //! Current mHelperTop matrix row
    int mHelperTopRow;

    //! Number of mCPMatrix columns
    int mCPCols;
    //! Number of mCPMatrix rows
    int mCPRows;

    //! Maximum tolerance in destination units
    double mSqrTolerance;

    //! Maximum source resolution
    double mMaxSrcXRes;
    double mMaxSrcYRes;

};

/// @endcond

#endif

