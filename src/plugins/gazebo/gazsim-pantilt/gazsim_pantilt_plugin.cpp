/***************************************************************************
 *  gazsim_pantilt_plugin.cpp - Plugin used to simulate a pantilt
 *
 *  Created: Thu Feb 25 09:18:05 2016
 *  Copyright  2015 Frederik Zwilling
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include <core/plugin.h>

//#include "pantilt_thread.h"
#include "gazsim_pantilt_thread.h"

using namespace fawkes;

/** Plugin to simulate pantilt
 * @author Frederik Zwilling
 */
class GazsimPantiltPlugin : public fawkes::Plugin
{
  public:
    /** Constructor.
     * @param config Fawkes configuration
     */
    GazsimPantiltPlugin(Configuration *config)
      : Plugin(config)
    {
      thread_list.push_back(new GazsimPantiltThread());
    }

};

PLUGIN_DESCRIPTION("Simulation of a Pantilt")
EXPORT_PLUGIN(GazsimPantiltPlugin)
