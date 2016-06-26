#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#include "../../interface/viewport.h"
#include "../../paint/paint.h"
#include "../../paint/supports.h"
#include "../track_paint.h"

enum {
	SPR_MOTION_SIMULATOR_STAIRS_R0		= 22154,
	SPR_MOTION_SIMULATOR_STAIRS_R1		= 22155,
	SPR_MOTION_SIMULATOR_STAIRS_R2		= 22156,
	SPR_MOTION_SIMULATOR_STAIRS_R3		= 22157,
	SPR_MOTION_SIMULATOR_STAIRS_RAIL_R0	= 22158,
	SPR_MOTION_SIMULATOR_STAIRS_RAIL_R1	= 22159,
	SPR_MOTION_SIMULATOR_STAIRS_RAIL_R2	= 22160,
	SPR_MOTION_SIMULATOR_STAIRS_RAIL_R3	= 22161,
};

/**
 *
 *  rct2: 0x0076522A
 */
static void paint_motionsimulator_vehicle(sint8 offsetX, sint8 offsetY, uint8 direction, int height, rct_map_element* mapElement)
{
	rct_ride *ride = get_ride(mapElement->properties.track.ride_index);
	rct_ride_entry *rideEntry = get_ride_entry_by_ride(ride);

	rct_map_element * savedMapElement = RCT2_GLOBAL(0x009DE578, rct_map_element*);

	rct_vehicle *vehicle = NULL;
	if (ride->lifecycle_flags & RIDE_LIFECYCLE_ON_TRACK) {
		uint16 spriteIndex = ride->vehicles[0];
		if (spriteIndex != SPRITE_INDEX_NULL) {
			vehicle = GET_VEHICLE(spriteIndex);
			RCT2_GLOBAL(0x009DE570, uint8) = 2;
			RCT2_GLOBAL(0x009DE578, rct_vehicle*) = vehicle;
		}
	}

	uint32 simulatorImageId = rideEntry->vehicles[0].base_image_id + direction;
	if (vehicle != NULL) {
		if (vehicle->restraints_position >= 64) {
			simulatorImageId += (vehicle->restraints_position >> 6) << 2;
		} else {
			simulatorImageId += vehicle->vehicle_sprite_type * 4;
		}
	}

	uint32 imageColourFlags = RCT2_GLOBAL(0x00F441A0, uint32);
	if (imageColourFlags == 0x20000000) {
		imageColourFlags = (uint32)(IMAGE_TYPE_UNKNOWN | IMAGE_TYPE_USE_PALETTE) << 28;
		imageColourFlags |= ride->vehicle_colours[0].trim_colour << 19;
		imageColourFlags |= ride->vehicle_colours[0].body_colour << 24;
	}
	simulatorImageId |= imageColourFlags;

	sint16 offsetZ = height + 2;
	uint32 imageId;
	uint8 currentRotation = get_current_rotation();
	switch (direction) {
	case 0:
		// Simulator
		imageId = simulatorImageId;
		sub_98197C(imageId, offsetX, offsetY, 20, 20, 44, offsetZ, offsetX, offsetY, offsetZ, currentRotation);
		// Stairs
		imageId = (SPR_MOTION_SIMULATOR_STAIRS_R0 + direction) | RCT2_GLOBAL(0x00F441A0, uint32);
		sub_98199C(imageId, offsetX, offsetY, 20, 20, 44, offsetZ, offsetX, offsetY, offsetZ, currentRotation);
		// Stairs (rail)
		imageId = (SPR_MOTION_SIMULATOR_STAIRS_RAIL_R0 + direction) | RCT2_GLOBAL(0x00F441A0, uint32);
		sub_98197C(imageId, offsetX, offsetY, 20, 2, 44, offsetZ, offsetX, offsetY + 32, offsetZ, currentRotation);
		break;
	case 1:
		// Simulator
		imageId = simulatorImageId;
		sub_98197C(imageId, offsetX, offsetY, 20, 20, 44, offsetZ, offsetX, offsetY, offsetZ, currentRotation);
		// Stairs
		uint32 imageId = (SPR_MOTION_SIMULATOR_STAIRS_R0 + direction) | RCT2_GLOBAL(0x00F441A0, uint32);
		sub_98199C(imageId, offsetX, offsetY, 20, 20, 44, offsetZ, offsetX, offsetY, offsetZ, currentRotation);
		// Stairs (rail)
		imageId = (SPR_MOTION_SIMULATOR_STAIRS_RAIL_R0 + direction) | RCT2_GLOBAL(0x00F441A0, uint32);
		sub_98197C(imageId, offsetX, offsetY, 2, 20, 44, offsetZ, offsetX + 34, offsetY, offsetZ, currentRotation);
		break;
	case 2:
		// Stairs (rail)
		imageId = (SPR_MOTION_SIMULATOR_STAIRS_RAIL_R0 + direction) | RCT2_GLOBAL(0x00F441A0, uint32);
		sub_98197C(imageId, offsetX, offsetY, 20, 2, 44, offsetZ, offsetX, offsetY - 10, offsetZ, currentRotation);
		// Stairs
		imageId = (SPR_MOTION_SIMULATOR_STAIRS_R0 + direction) | RCT2_GLOBAL(0x00F441A0, uint32);
		sub_98197C(imageId, offsetX, offsetY, 20, 20, 44, offsetZ, offsetX, offsetY + 5, offsetZ, currentRotation);
		// Simulator
		imageId = simulatorImageId;
		sub_98199C(imageId, offsetX, offsetY, 20, 20, 44, offsetZ, offsetX, offsetY + 5, offsetZ, currentRotation);
		break;
	case 3:
		// Stairs (rail)
		imageId = (SPR_MOTION_SIMULATOR_STAIRS_RAIL_R0 + direction) | RCT2_GLOBAL(0x00F441A0, uint32);
		sub_98197C(imageId, offsetX, offsetY, 2, 20, 44, offsetZ, offsetX - 10, offsetY, offsetZ, currentRotation);
		// Stairs
		imageId = (SPR_MOTION_SIMULATOR_STAIRS_R0 + direction) | RCT2_GLOBAL(0x00F441A0, uint32);
		sub_98197C(imageId, offsetX, offsetY, 20, 20, 44, offsetZ, offsetX + 5, offsetY, offsetZ, currentRotation);
		// Simulator
		imageId = simulatorImageId;
		sub_98199C(imageId, offsetX, offsetY, 20, 20, 44, offsetZ, offsetX + 5, offsetY, offsetZ, currentRotation);
		break;
	}

	RCT2_GLOBAL(0x009DE578, rct_map_element*) = savedMapElement;
	gPaintInteractionType = VIEWPORT_INTERACTION_ITEM_RIDE;
}

/** rct2: 0x008A85C4 */
static void paint_motionsimulator(uint8 rideIndex, uint8 trackSequence, uint8 direction, int height, rct_map_element * mapElement)
{
	trackSequence = track_map_2x2[direction][trackSequence];

	int edges = edges_2x2[trackSequence];
	rct_ride *ride = get_ride(rideIndex);
	rct_xy16 position = { RCT2_GLOBAL(0x009DE56A, sint16), RCT2_GLOBAL(0x009DE56E, sint16) };

	wooden_a_supports_paint_setup((direction & 1), 0, height, RCT2_GLOBAL(0x00F441A0, uint32), NULL);
	track_paint_util_paint_floor(edges, RCT2_GLOBAL(0x00F44198, uint32), height, floorSpritesCork, get_current_rotation());
	track_paint_util_paint_fences(edges, position, mapElement, ride, RCT2_GLOBAL(0x00F4419C, uint32), height, fenceSpritesRope, get_current_rotation());

	switch (trackSequence) {
	case 1: paint_motionsimulator_vehicle( 16, -16, direction, height, mapElement); break;
	case 2: paint_motionsimulator_vehicle(-16,  16, direction, height, mapElement); break;
	case 3: paint_motionsimulator_vehicle(-16, -16, direction, height, mapElement); break;
	}

	paint_util_set_all_segments_support_invalid_height();
	paint_util_set_general_support_height(height + 128, 0x20);
}

/**
 *
 *  rct2: 0x00763520
 */
TRACK_PAINT_FUNCTION get_track_paint_function_motionsimulator(int trackType, int direction)
{
	switch (trackType) {
	case 110: return paint_motionsimulator;
	}
	return NULL;
}
