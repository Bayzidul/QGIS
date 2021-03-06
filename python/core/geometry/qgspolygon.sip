class QgsPolygonV2: public QgsCurvePolygon
{
%TypeHeaderCode
#include <qgspolygon.h>
%End

  public:
    QgsPolygonV2();

    bool operator==( const QgsPolygonV2& other ) const;
    bool operator!=( const QgsPolygonV2& other ) const;

    virtual QString geometryType() const;
    virtual QgsPolygonV2* clone() const /Factory/;
    void clear();

    virtual bool fromWkb( QgsConstWkbPtr& wkb );

    // inherited: bool fromWkt( const QString &wkt );

    QByteArray asWkb() const;
    // inherited: QString asWkt( int precision = 17 ) const;
    // inherited: QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    // inherited: QString asJSON( int precision = 17 ) const;

    QgsPolygonV2* surfaceToPolygon() const /Factory/;

    /** Returns the geometry converted to the more generic curve type QgsCurvePolygon
     @return the converted geometry. Caller takes ownership*/
    QgsAbstractGeometry* toCurveType() const /Factory/;

    void addInteriorRing( QgsCurve* ring /Transfer/ );
    //overridden to handle LineString25D rings
    virtual void setExteriorRing( QgsCurve* ring /Transfer/ );

    virtual QgsAbstractGeometry* boundary() const /Factory/;

    /**
     * Returns the distance from a point to the boundary of the polygon (either the
     * exterior ring or any closer interior rings). The returned distance will be
     * negative if the point lies outside the polygon.
     * @note added in QGIS 3.0
     */
    double pointDistanceToBoundary( double x, double y ) const;
};
