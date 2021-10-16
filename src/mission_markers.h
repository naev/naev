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
   SYSMARKER_PLOT,      /**< Marker is for plot priority (ultra high) mission targets. */
   PNTMARKER_COMPUTER,  /**< Marker is for mission computer missions. */
   PNTMARKER_LOW,       /**< Marker is for low priority planet targets. */
   PNTMARKER_HIGH,      /**< Marker is for high priority planet targets. */
   PNTMARKER_PLOT,      /**< Marker is for plot priority planet targets. */
} MissionMarkerType;

/**
 * @brief Mission system marker.
 */
typedef struct MissionMarker_ {
   int id;                 /**< ID of the mission marker. */
   int objid;              /**< ID of marked system. */
   MissionMarkerType type; /**< Marker type. */
} MissionMarker;
