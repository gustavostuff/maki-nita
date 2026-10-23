# Snake entities

Snake is a later cart. The head is one 1 by 1 entity and it can collide. The body is a set of straight runs. Each run is a 1 by N or N by 1 line, and the runs change as the snake moves.

Bitmap shapes stay at most 4 by 4. The picture is one 16-bit word in flash, so a long line does not fit in that word. A snake run also changes on every move, so a general bitmap for it would live in RAM and the pixel tests would grow with the area of the rectangle.

A line is a length and a direction. Drawing it, and testing it against the head, walks one axis. That form can sit beside the bitmap shapes when this cart is built. The entity cap is 64. A cart may use fewer. A run that turns on every cell spends one entity per cell, so a snake of more than about 64 cells that turns on every cell does not fit. A snake that holds a straight stretch spends one entity for that whole stretch. The head is the entity that collides. The runs are the entities that can be collided with.

The space shooter keeps its own budget inside that table. That budget is at most 32 rocks, 8 bullets, and the ship.
