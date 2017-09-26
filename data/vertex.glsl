/*
Copyright (C) 2016 Jeremy Starnes

This file is part of Transport RTS.

Transport RTS is free software: you can redistribute it and/or modify it under
the terms of the GNU Affero General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

Transport RTS is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more
details.

You should have received a copy of the GNU Affero General Public License along
with Transport RTS; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/agpl>.
*/



#version 120
// in/out and layout(location) do not exist!

uniform mat3 mvp;
attribute vec2 vertex;

void main() {
  gl_Position = vec4(
    ( mvp*vec3(vertex, 1.0) ).xy,
    1.0,
    1.0
  );
}
