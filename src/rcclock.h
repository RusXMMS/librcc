/*
  LibRCC - module responsible for application synchronization (write access to
  a shared config mainly)

  Copyright (C) 2005-2008 Suren A. Chilingaryan <csa@dside.dyndns.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as published
  by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/

#ifndef _RCC_LOCK_H
#define _RCC_LOCK_H

#ifndef LIBRCC_LOCK_WAIT
# define LIBRCC_LOCK_WAIT 3000 /* ms */
#endif /* LIBRCC_LOCK_WAIT */

int rccLock();
void rccUnLock();

#endif /* _RCC_LOCK_H */

