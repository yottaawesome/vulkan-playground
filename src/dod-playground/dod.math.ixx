export module dod:math;

import std;

export namespace dod
{
    // Tiny math. Intentionally minimal — DOD is about data layout, not linear algebra.
    struct Vec3
    {
        float x{}, y{}, z{};

        constexpr Vec3 operator-(const Vec3& o) const noexcept { return { x - o.x, y - o.y, z - o.z }; }
        constexpr float Dot(const Vec3& o) const noexcept { return x * o.x + y * o.y + z * o.z; }
        constexpr float LengthSq() const noexcept { return Dot(*this); }
    };

    // Six-plane frustum in the form plane.xyz * p + plane.w >= 0 is "inside".
    struct Plane
    {
        Vec3 normal;
        float d;
    };

    struct Frustum
    {
        std::array<Plane, 6> planes;

        // Sphere-vs-frustum test. Returns true if the sphere is at least partially inside.
        bool ContainsSphere(const Vec3& center, float radius) const noexcept
        {
            for (const auto& p : planes)
            {
                const float signedDist = p.normal.Dot(center) + p.d;
                if (signedDist < -radius)
                    return false;
            }
            return true;
        }
    };

    // A boring but well-conditioned cube frustum centred at origin, half-extent = size.
    inline Frustum MakeBoxFrustum(float size) noexcept
    {
        return Frustum{ {{
            Plane{ { 1,  0,  0}, size },
            Plane{ {-1,  0,  0}, size },
            Plane{ { 0,  1,  0}, size },
            Plane{ { 0, -1,  0}, size },
            Plane{ { 0,  0,  1}, size },
            Plane{ { 0,  0, -1}, size },
        }} };
    }
}
