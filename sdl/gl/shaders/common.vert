#version 300 es
const
float glPI = 3.14159265358979;

/* The simplest way to make z affect w */
const
mat4 perspective = mat4(1.0, 0.0,  0.0, 0.0,
                        0.0, 1.0,  0.0, 0.0,
                        0.0, 0.0,  1.0, 0.0,
                        0.0, 0.0, -1.0, 1.0);

mat4 rotatex(float deg)
{
    float rad = deg * glPI / 180.f;
    return mat4(1.0, 0.0, 0.0, 0.0,
                0.0, cos(rad), -sin(rad), 0.0,
                0.0, sin(rad),  cos(rad), 0.0,
                0.0, 0.0,  0.0, 1.0);
}

mat4 rotatey(float deg)
{
    float rad = deg * glPI / 180.f;
    return mat4(cos(rad), 0.0, -sin(rad), 0.0,
                0.0, 1.0, 0.0,  0.0,
                sin(rad), 0.0,  cos(rad), 0.0,
                0.0, 0.0, 0.0,  1.0);
}

mat4 rotatez(float deg)
{
    float rad = deg * glPI / 180.f;
    return mat4(cos(rad), -sin(rad), 0.0, 0.0,
                sin(rad),  cos(rad), 0.0, 0.0,
                0.0, 0.0,  1.0, 0.0,
                0.0, 0.0,  0.0, 1.0);
}

mat4 move_xyz(vec3 m)
{
    return mat4(1.0, 0.0, 0.0, m.x,
                0.0, 1.0, 0.0, m.y,
                0.0, 0.0, 1.0, m.z,
                0.0, 0.0, 0.0, 1.0);
}

mat4 scale_xyz(vec3 s)
{
    return mat4(s.x, 0.0, 0.0, 0.0,
                0.0, s.y, 0.0, 0.0,
                0.0, 0.0, s.z, 0.0,
                0.0, 0.0, 0.0, 1.0);
}

