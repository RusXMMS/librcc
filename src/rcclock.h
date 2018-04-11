/*
  LibRCC - module responsible for application synchronization (write access to
  a shared config mainly)

  Copyright (C) 2005-2018 Suren A. Chilingaryan <csa@suren.me>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License version 2.1 or later
  as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
  for more details.

  You should have received a copy of the GNU Lesser General Public License 
  along with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef _RCC_LOCK_H
#define _RCC_LOCK_H

#ifndef LIBRCC_LOCK_WAIT
# define LIBRCC_LOCK_WAIT 3000 /* ms */
#endif /* LIBRCC_LOCK_WAIT */

int rccLock();
void rccUnLock();

#endif /* _RCC_LOCK_H */

