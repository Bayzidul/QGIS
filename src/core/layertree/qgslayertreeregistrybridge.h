/***************************************************************************
  qgslayertreeregistrybridge.h
  --------------------------------------
  Date                 : May 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEREGISTRYBRIDGE_H
#define QGSLAYERTREEREGISTRYBRIDGE_H

#include <QObject>
#include <QStringList>

#include "qgis_core.h"

class QgsLayerTreeGroup;
class QgsLayerTreeNode;
class QgsMapLayer;
class QgsProject;


/** \ingroup core
 * Listens to the updates in map layer registry and does changes in layer tree.
 *
 * When connected to a layer tree, any layers added to the map layer registry
 * will be also added to the layer tree. Similarly, map layers that are removed
 * from registry will be removed from the layer tree.
 *
 * If a layer is completely removed from the layer tree, it will be also removed
 * from the map layer registry.
 *
 * @note added in 2.4
 */
class CORE_EXPORT QgsLayerTreeRegistryBridge : public QObject
{
    Q_OBJECT
  public:
    //! Create the instance that synchronizes given project with a layer tree root
    explicit QgsLayerTreeRegistryBridge( QgsLayerTreeGroup* root, QgsProject* project, QObject *parent = nullptr );

    void setEnabled( bool enabled ) { mEnabled = enabled; }
    bool isEnabled() const { return mEnabled; }

    void setNewLayersVisible( bool enabled ) { mNewLayersVisible = enabled; }
    bool newLayersVisible() const { return mNewLayersVisible; }

    //! Set where the new layers should be inserted - can be used to follow current selection.
    //! By default it is root group with zero index.
    void setLayerInsertionPoint( QgsLayerTreeGroup* parentGroup, int index );

  signals:
    //! Tell others we have just added layers to the tree (used in QGIS to auto-select first newly added layer)
    //! @note added in 2.6
    void addedLayersToLayerTree( const QList<QgsMapLayer*>& layers );

  protected slots:
    void layersAdded( const QList<QgsMapLayer*>& layers );
    void layersWillBeRemoved( const QStringList& layerIds );

    void groupWillRemoveChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void groupRemovedChildren();

    void removeLayersFromRegistry( const QStringList& layerIds );

  protected:
    QgsLayerTreeGroup* mRoot = nullptr;
    QgsProject* mProject = nullptr;
    QStringList mLayerIdsForRemoval;
    bool mRegistryRemovingLayers;
    bool mEnabled;
    bool mNewLayersVisible;

    QgsLayerTreeGroup* mInsertionPointGroup = nullptr;
    int mInsertionPointIndex;
};

#endif // QGSLAYERTREEREGISTRYBRIDGE_H
