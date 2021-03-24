/* 
 * This file is part of the rpcbuf distribution (https://github.com/tomwimmenhove/rpcbuf).
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

DEFINE_CALL_START;
//DEFINE_CALL(foo, double(int, float, double));
//DEFINE_CALL(foo2, int(int, int, int));
//DEFINE_CALL(bar, void());
DEFINE_CALL(putc, void(char));
DEFINE_CALL(puts, void(const char*));
DEFINE_CALL(dint, void(uint64_t));
DEFINE_CALL(pxint, void(uint64_t));
DEFINE_CALL(pdint, void(uint64_t));
DEFINE_CALL_END;
