float hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}

float valueNoise(float2 p)
{
    float2 cell = floor(p);
    float2 local = frac(p);
    local = local * local * (3.0 - 2.0 * local);
    float a = hash21(cell);
    float b = hash21(cell + float2(1.0, 0.0));
    float c = hash21(cell + float2(0.0, 1.0));
    float d = hash21(cell + float2(1.0, 1.0));
    return lerp(lerp(a, b, local.x), lerp(c, d, local.x), local.y);
}

float fractalNoise(float2 p)
{
    float value = 0.0;
    float amplitude = 0.5;
    [unroll]
    for (int octave = 0; octave < 4; ++octave)
    {
        value += amplitude * valueNoise(p);
        p = p * 2.03 + 17.1;
        amplitude *= 0.5;
    }
    return value;
}

float liquidHeight(float2 p, float seed)
{
    float2 warp = float2(fractalNoise(p + seed), fractalNoise(p + float2(5.2, 1.3) + seed));
    return fractalNoise(p + 2.2 * warp);
}

float environment(float2 direction)
{
    float stripe = 0.5 + 0.8 * direction.y + 0.45 * sin(direction.x * 3.0 + direction.y * 1.5);
    return pow(saturate(stripe), 1.8);
}

float3 liquidChrome(float2 p, float seed, float dispersion, float2 extraGradient)
{
    const float epsilon = 0.012;
    float2 gradient = float2(liquidHeight(p + float2(epsilon, 0.0), seed) - liquidHeight(p - float2(epsilon, 0.0), seed),
                             liquidHeight(p + float2(0.0, epsilon), seed) - liquidHeight(p - float2(0.0, epsilon), seed)) /
                      (2.0 * epsilon) +
                      extraGradient;

    float3 channels;
    [unroll]
    for (int channel = 0; channel < 3; ++channel)
    {
        float scale = 1.0 + (channel - 1) * dispersion;
        float3 normal = normalize(float3(-gradient * 0.2 * scale, 1.0));
        float3 reflected = reflect(float3(0.0, 0.0, -1.0), normal);
        channels[channel] = environment(reflected.xy);
    }
    return channels;
}
