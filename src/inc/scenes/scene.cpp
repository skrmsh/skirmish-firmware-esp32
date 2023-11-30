/*
Skirmish ESP32 Firmware

Scene base class

Copyright (C) 2023 Ole Lange
*/

#include "scene.h"

/**
 * Constructor
 */
SkirmishUIScene::SkirmishUIScene(SkirmishUI *ui) { this->ui = ui; }

/**
 * Is called when the scene is set
 */
void SkirmishUIScene::onSet(uint8_t id) { this->id = id; }

/**
 * Update function. Should be used to update/prepare all
 * data required to render this scene. The return value
 * is indicating if rendering is required, but if it's false
 * it might be rendered sometimes anyways.
 *
 * @return a boolean value indicating if rendering is required
 */
bool SkirmishUIScene::update() { return false; }

void SkirmishUIScene::render() {}

uint8_t SkirmishUIScene::getID() { return id; }
