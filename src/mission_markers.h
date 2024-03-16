/*
 * See Licensing and Copyright notice in naev.h
 */
#pragma once

/**
 * @brief Different type of markers.
 */
typedef enum MissionMarkerType_ {
   SYSMARKER_COMPUTER,  /**< Marker is for mission computer missions. */
   SYSMARKER_LOW,       /**< Marker is for low priority mission targets. */
   SYSMARKER_HIGH,      /**< Marker is for high priority mission targets. */
   SYSMARKER_PLOT,      /**< Marker is for plot priority (ultra high) mission
                           targets. */
   SPOBMARKER_COMPUTER, /**< Marker is for mission computer missions. */
   SPOBMARKER_LOW,      /**< Marker is for low priority spob targets. */
   SPOBMARKER_HIGH,     /**< Marker is for high priority spob targets. */
   SPOBMARKER_PLOT,     /**< Marker is for plot priority spob targets. */
} MissionMarkerType;

/**
 * @brief Mission system marker.
 */
typedef struct MissionMarker_ {
   int               id;    /**< ID of the mission marker. */
   int               objid; /**< ID of marked system. */
   MissionMarkerType type;  /**< Marker type. */
} MissionMarker;
