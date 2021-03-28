/* 
 * This file is part of the alcatraz distribution (https://github.com/tomwimmenhove/alcatraz);
 * Copyright (c) 2021 Tom Wimmenhove.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DUMMY_H
#define DUMMY_H

#ifdef GUEST_CUSTOM_WRITE
#define GUEST_CUSTOM_WRITE_CODE
#else
#define GUEST_CUSTOM_WRITE_CODE int64_t write(int fd, const void *buf, size_t count) { return 0; }
#endif

#ifdef GUEST_CUSTOM_READ
#define GUEST_CUSTOM_READ_CODE
#else
#define GUEST_CUSTOM_READ_CODE int64_t read(int fd, const void *buf, size_t count) { return 0; }
#endif

#ifdef GUEST_CUSTOM_CLOSE
#define GUEST_CUSTOM_CLOSE_CODE
#else
#define GUEST_CUSTOM_CLOSE_CODE int close(int) { return 0; }
#endif

#define DEFINE_GUEST_DUMMY_CALLS	\
extern "C" {						\
	GUEST_CUSTOM_WRITE_CODE			\
	GUEST_CUSTOM_READ_CODE			\
	GUEST_CUSTOM_CLOSE_CODE			\
}

#endif /* DUMMY_H */
