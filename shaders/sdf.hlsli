static const float kPi = 3.14159265358979;

float cornerRadiusAt(float2 fromCenter, float4 radii)
{
    float2 side = fromCenter.x > 0.0 ? radii.yz : radii.xw;
    return fromCenter.y > 0.0 ? side.y : side.x;
}

float roundedBoxDistance(float2 fromCenter, float2 halfSize, float radius)
{
    float2 q = abs(fromCenter) - halfSize + radius;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;
}

float2 erfApproximation(float2 x)
{
    float2 sign_ = sign(x);
    float2 magnitude = abs(x);
    float2 t = 1.0 + (0.278393 + (0.230389 + 0.078108 * (magnitude * magnitude)) * magnitude) * magnitude;
    t *= t;
    return sign_ - sign_ / (t * t);
}

float gaussian(float x, float sigma)
{
    return exp(-(x * x) / (2.0 * sigma * sigma)) / (sqrt(2.0 * kPi) * sigma);
}

float roundedBoxShadowRow(float x, float y, float sigma, float corner, float2 halfSize)
{
    float delta = min(halfSize.y - corner - abs(y), 0.0);
    float curved = halfSize.x - corner + sqrt(max(0.0, corner * corner - delta * delta));
    float2 integral = 0.5 + 0.5 * erfApproximation((x + float2(-curved, curved)) * (sqrt(0.5) / sigma));
    return integral.y - integral.x;
}

float roundedBoxShadow(float2 fromCenter, float2 halfSize, float sigma, float corner)
{
    float low = fromCenter.y - halfSize.y;
    float high = fromCenter.y + halfSize.y;
    float start = clamp(-3.0 * sigma, low, high);
    float end = clamp(3.0 * sigma, low, high);
    float step_ = (end - start) / 4.0;
    float y = start + step_ * 0.5;
    float value = 0.0;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        value += roundedBoxShadowRow(fromCenter.x, fromCenter.y - y, sigma, corner, halfSize) * gaussian(y, sigma) * step_;
        y += step_;
    }
    return value;
}
