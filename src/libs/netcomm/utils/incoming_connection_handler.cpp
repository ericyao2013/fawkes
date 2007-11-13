
/***************************************************************************
 *  connection_handler.h - handler managing incoming connections
 *
 *  Created: Tue Nov 13 16:17:34 2007
 *  Copyright  2006-2007  Tim Niemueller [www.niemueller.de]
 *
 *  $Id$
 *
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1307, USA.
 */

#include <netcomm/utils/incoming_connection_handler.h>

/** @class NetworkIncomingConnectionHandler <netcomm/utils/incoming_connection_handler.h>
 * Interface for handling incoming connections.
 * @author Tim Niemueller
 *
 * @fn void NetworkIncomingConnectionHandler::add_connection(StreamSocket *s) throw() = 0
 * Add an incoming connection.
 * This is called for instance by the NetworkAcceptorThread whenever a new connection has
 * been accepted.
 * @param s socket for new connection
 */

/** Virtual destructor. */
NetworkIncomingConnectionHandler::~NetworkIncomingConnectionHandler()
{
}

