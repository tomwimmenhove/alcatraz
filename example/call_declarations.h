/* 
 * This file is part of the alcatraz distribution (https://github.com/tomwimmenhove/alcatraz).
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

DEFINE_CALL_START(receiver);
DEFINE_CALL(putc, void(char));
DEFINE_CALL(dint, void(uint64_t));
DEFINE_CALL(pxint, void(uint64_t));
DEFINE_CALL(pdint, void(uint64_t));
DEFINE_CALL(write, size_t(int, const void*, size_t));

DEFINE_CALL(set_msg_pump_ready, void(bool));

DEFINE_CALL(set_test_address, void(uintptr_t fn));

DEFINE_CALL(wait, void());

DEFINE_CALL_END;
