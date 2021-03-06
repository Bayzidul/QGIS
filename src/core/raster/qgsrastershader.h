/* **************************************************************************
                qgsrastershader.h -  description
                       -------------------
begin                : Fri Dec 28 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSRASTERSHADER_H
#define QGSRASTERSHADER_H

#include "qgis_core.h"

class QDomDocument;
class QDomElement;
class QgsRasterShaderFunction;

/** \ingroup core
 * Interface for all raster shaders.
 */
class CORE_EXPORT QgsRasterShader
{

  public:
    QgsRasterShader( double minimumValue = 0.0, double maximumValue = 255.0 );
    ~QgsRasterShader();

    //! QgsRasterShader cannot be copied
    QgsRasterShader( const QgsRasterShader& rh ) = delete;
    //! QgsRasterShader cannot be copied
    QgsRasterShader& operator=( const QgsRasterShader& rh ) = delete;

    /*
     *
     * Non-Static Inline methods
     *
     */
    //! \brief Return the maximum value for the raster shader
    double maximumValue() { return mMaximumValue; }

    //! \brief Return the minimum value for the raster shader
    double minimumValue() { return mMinimumValue; }

    QgsRasterShaderFunction* rasterShaderFunction() { return mRasterShaderFunction; }
    const QgsRasterShaderFunction* rasterShaderFunction() const { return mRasterShaderFunction; }

    /*
     *
     * Non-Static methods
     *
     */
    //! \brief generates and new RGBA value based on one input value
    bool shade( double, int*, int*, int*, int* );

    //! \brief generates and new RGBA value based on original RGBA value
    bool shade( double, double, double, double, int*, int*, int*, int* );

    /** \brief A public method that allows the user to set their own shader function
      \note Raster shader takes ownership of the shader function instance */
    void setRasterShaderFunction( QgsRasterShaderFunction* );

    //! \brief Set the maximum value
    void setMaximumValue( double );

    //! \brief Return the minimum value
    void setMinimumValue( double );

    /**
     * Writes shader state to an XML element.
     */
    void writeXml( QDomDocument& doc, QDomElement& parent ) const;

    /**
     * Reads shader state from an XML element.
     */
    void readXml( const QDomElement& elem );

  private:
    //! \brief User defineable minimum value for the raster shader
    double mMinimumValue;

    //! \brief user defineable maximum value for the raster shader
    double mMaximumValue;

    //! \brief Pointer to the shader function
    QgsRasterShaderFunction* mRasterShaderFunction = nullptr;

};
#endif
