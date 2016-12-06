










#ifndef TRACE_H
#define TRACE_H

class Vec2i;
class Unit;
class Building;

int32_t Trace(int32_t utype, int32_t umode,
          Vec2i vstart, Vec2i vend,
          Unit* thisu, Unit* targu, Building* targb);

#endif
