/*
  ==============================================================================

    BandControlComponent.cpp
    Created: 4 Jul 2025 11:15:15am
    Author:  Brandon Nissl

  ==============================================================================
*/

#include "BandControlComponent.h"

void BandControlComponent::resized()
{
    auto area = getLocalBounds();
    auto bandWidth = area.getWidth() / 4;

    lowBand.setBounds(area.removeFromLeft(bandWidth));
    midBand.setBounds(area.removeFromLeft(bandWidth));
    highBand.setBounds(area.removeFromLeft(bandWidth));
    airBand.setBounds(area);
}
