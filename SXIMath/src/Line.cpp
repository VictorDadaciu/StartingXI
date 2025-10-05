#include "Line.h"

namespace sxi 
{
    Line::Line(const glm::vec2 &start, const glm::vec2 &end) : start(start), end(end) {}

    bool Line::intersectsOneWay(const Line &other) const 
    {
        glm::vec2 axisProj = glm::vec2(-end.y + start.y, end.x - start.x);
        float aMin, aMax, bMin, bMax;
        {
            float q1 = glm::dot(axisProj, start);
            float q2 = glm::dot(axisProj, end);
            aMin = fminf(q1, q2);
            aMax = fmaxf(q1, q2);
        }
        {
            float q1 = glm::dot(axisProj, start);
            float q2 = glm::dot(axisProj, end);
            bMin = fminf(q1, q2);
            bMax = fmaxf(q1, q2);
        }
        return aMax >= bMin && bMax >= aMin;
    }

    bool Line::intersects(const Line &other) const 
    {
        return intersectsOneWay(other) && other.intersectsOneWay(*this);
    }

    float Line::sqrDistToClosestPoint(const glm::vec2& point) const
    {
        glm::vec2 v = end - start;
        glm::vec2 u = point - start;
        float t = glm::dot(v, u) / glm::sqrLength(v);
        if (t >= 0 && t <= 1)
            return glm::sqrLength((start * (1 - t) + end * t) - point);
        float dStart = glm::sqrLength(u);
        float dEnd = glm::sqrLength(end - point);
        return fminf(dStart, dEnd);
    }

    glm::vec2 Line::closestNewPointOutsideAndSqrDist(const glm::vec2& point, float& sqrDist) const
    {
        glm::vec2 line = end - start;
        glm::vec2 fromStart = point - start;
        float t = glm::dot(line, fromStart) / glm::sqrLength(line);
        if (t >= 0 && t <= 1)
        {
            glm::vec2 toNewPoint = 1.01f * (start * (1 - t) + end * t - point);
            sqrDist = glm::sqrLength(toNewPoint);
            return point + toNewPoint;
        }
        glm::vec2 toStart = -1.01f * fromStart;
        glm::vec2 toEnd = 1.01f * (end - point);
        float dStart = glm::sqrLength(toStart);
        float dEnd = glm::sqrLength(toEnd);
        sqrDist = fminf(dStart, dEnd);
        return point + (dStart < dEnd ? toStart : toEnd);
    }

    Ray::Ray(const glm::vec2 &origin, const glm::vec2 &dir) : origin(origin), dir(dir) {}

    bool Ray::intersects(const Line &line, glm::vec2 &point) const 
    {
        glm::vec2 v1 = origin - line.start;
        glm::vec2 v2 = line.end - line.start;
        glm::vec2 v3(-dir.y, dir.x);
        float dot = glm::dot(v2, v3);
        float t1 = glm::cross(v1, v1) / dot;
        float t2 = glm::dot(v1, v3) / dot;
        point = line.start + v2 * t2;
        return t1 >= 0 && 0 <= t1 && t1 <= 1;
    }

    bool Ray::between(const Ray &left, const Ray &right) const 
    {
        return glm::cross(left.dir, dir) * glm::cross(right.dir, dir) <= 0;
    }
}