#include "hero.hlsli"
#include "sdf.hlsli"

cbuffer FrameConstants : register(b0)
{
    float2 stageSize;
    uint baseInstance;
    uint padding;
};

cbuffer BackdropField : register(b1)
{
    float2 fieldPosition;
    float2 fieldVelocity;
    float fieldPresence;
    float fieldRadius;
    float2 fieldTrail;
    float4 fieldRipples[4];
};

struct ShapeInstance
{
    float4 rect;
    float4 radii;
    float4 fill;
    float4 border;
    float4 clip;
    float4 params0;
    float4 params1;
    float4 params2;
};

StructuredBuffer<ShapeInstance> instances : register(t0);
Texture2D<float4> glyphAtlas : register(t1);
Texture2D<float4> sceneTexture : register(t2);
Texture2DArray<float4> artImages : register(t3);
Texture2DArray<float4> flagImages : register(t4);
SamplerState glyphSampler : register(s0);

static const uint kindRect = 0;
static const uint kindShadow = 1;
static const uint kindMask = 2;
static const uint kindGlyph = 3;
static const uint kindGlass = 4;
static const uint kindHero = 5;
static const uint kindArt = 6;
static const uint kindFlag = 7;

struct VertexOutput
{
    float4 position : SV_Position;
    float2 pixel : TEXCOORD0;
    nointerpolation uint instanceIndex : TEXCOORD1;
};

VertexOutput vsMain(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
{
    uint index = baseInstance + instanceId;
    ShapeInstance shape = instances[index];

    float expand = shape.params0.z;
    float2 corner = float2(vertexId & 1, vertexId >> 1);
    float2 pixel = shape.rect.xy - expand + corner * (shape.rect.zw + expand * 2.0);

    VertexOutput output;
    output.position = float4(pixel / stageSize * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    output.pixel = pixel;
    output.instanceIndex = index;
    return output;
}

float4 shadeRect(ShapeInstance shape, float2 fromCenter, float2 halfSize)
{
    float radius = cornerRadiusAt(fromCenter, shape.radii);
    float borderWidth = shape.params0.x;

    float outerDistance = roundedBoxDistance(fromCenter, halfSize, radius);
    float innerDistance = roundedBoxDistance(fromCenter, halfSize - borderWidth, max(radius - borderWidth, 0.0));
    float outerCoverage = saturate(0.5 - outerDistance);
    float innerCoverage = saturate(0.5 - innerDistance);
    float ring = outerCoverage - innerCoverage;

    float4 color = shape.fill * outerCoverage;
    return shape.border * ring + color * (1.0 - shape.border.a * ring);
}

float4 shadeShadow(ShapeInstance shape, float2 fromCenter, float2 halfSize)
{
    float sigma = shape.params1.x;
    return shape.fill * roundedBoxShadow(fromCenter, halfSize, sigma, shape.radii.x);
}

float4 shadeMask(ShapeInstance shape, float2 fromCenter, float2 halfSize)
{
    float radius = cornerRadiusAt(fromCenter, shape.radii);
    float coverage = saturate(0.5 - roundedBoxDistance(fromCenter, halfSize, radius));
    return float4(coverage, coverage, coverage, coverage);
}

float median(float3 value)
{
    return max(min(value.x, value.y), min(max(value.x, value.y), value.z));
}

float4 shadeGlyph(ShapeInstance shape, float2 pixel)
{
    float2 t = (pixel - shape.rect.xy) / shape.rect.zw;
    float2 uv = lerp(shape.radii.xy, shape.radii.zw, t);
    float signedDistance = median(glyphAtlas.SampleLevel(glyphSampler, uv, 0).rgb);
    float opacity = saturate(shape.params1.x * (signedDistance - 0.5) + 0.5);
    return shape.fill * opacity;
}


static const float2 kLightDirection = float2(-0.5547, -0.8321);

float2 roundedBoxNormal(float2 fromCenter, float2 halfSize, float radius)
{
    const float2 step_ = float2(0.5, 0.0);
    float dx = roundedBoxDistance(fromCenter + step_.xy, halfSize, radius) - roundedBoxDistance(fromCenter - step_.xy, halfSize, radius);
    float dy = roundedBoxDistance(fromCenter + step_.yx, halfSize, radius) - roundedBoxDistance(fromCenter - step_.yx, halfSize, radius);
    float2 gradient = float2(dx, dy);
    float length_ = length(gradient);
    return length_ > 1e-5 ? gradient / length_ : float2(0.0, 0.0);
}

float4 sampleScene(float2 pixel)
{
    return sceneTexture.SampleLevel(glyphSampler, pixel / stageSize, 0);
}

float4 shadeGlass(ShapeInstance shape, float2 pixel, float2 fromCenter, float2 halfSize)
{
    float4 destination = sceneTexture.Load(int3((int2)floor(pixel), 0));

    float radius = cornerRadiusAt(fromCenter, shape.radii);
    float edgeDistance = roundedBoxDistance(fromCenter, halfSize, radius);
    float coverage = saturate(0.5 - edgeDistance);
    if (coverage <= 0.0)
        return destination;

    float bevel = max(shape.params0.x, 1.0);
    float refraction = shape.params1.x;
    float dispersion = shape.params1.y;
    float specular = shape.params1.z;
    float frost = shape.params1.w;

    float depth = saturate(-edgeDistance / bevel);
    float edge = pow(1.0 - depth, 2.0);
    float2 normal = roundedBoxNormal(fromCenter, halfSize, radius);
    float2 offset = -normal * refraction * edge;

    static const float2 taps[5] = {float2(0.0, 0.0), float2(1.0, 0.0), float2(-1.0, 0.0), float2(0.0, 1.0), float2(0.0, -1.0)};
    float4 refracted = float4(0.0, 0.0, 0.0, 0.0);
    [unroll]
    for (int i = 0; i < 5; ++i)
    {
        float2 tap = taps[i] * frost;
        refracted.r += sampleScene(pixel + offset * (1.0 + dispersion * edge) + tap).r;
        refracted.g += sampleScene(pixel + offset + tap).g;
        refracted.b += sampleScene(pixel + offset * (1.0 - dispersion * edge) + tap).b;
        refracted.a += sampleScene(pixel + offset + tap).a;
    }
    refracted *= 0.2;

    float blur = shape.params2.y;
    if (blur > 0.0)
    {
        static const int blurTaps = 28;
        refracted = float4(0.0, 0.0, 0.0, 0.0);
        [unroll]
        for (int j = 0; j < blurTaps; ++j)
        {
            float angle = (float)j * 2.399963;
            float distance = sqrt(((float)j + 0.5) / (float)blurTaps) * blur;
            refracted += sampleScene(pixel + offset + float2(cos(angle), sin(angle)) * distance);
        }
        refracted /= (float)blurTaps;
    }

    float4 color = shape.fill + refracted * (1.0 - shape.fill.a);

    float lit = dot(normal, kLightDirection) * 0.5 + 0.5;
    float shade = (1.0 - lit) * pow(1.0 - depth, 2.0) * 0.35;
    color.rgb *= 1.0 - shade;

    float rimBand = saturate(1.25 - abs(edgeDistance + 0.75));
    float rimIntensity = lerp(0.35, 1.0, lit * lit) * rimBand;
    float4 rim = shape.border * rimIntensity;
    color = rim + color * (1.0 - rim.a);

    float highlight = specular * smoothstep(0.55, 1.0, lit) * pow(1.0 - depth, 1.6) * step(0.001, depth);
    color += float4(highlight, highlight, highlight, highlight);

    return lerp(destination, color, coverage * shape.params2.x);
}

struct Disturbance
{
    float2 warp;
    float2 gradient;
    float glow;
};

Disturbance backdropDisturbance(float2 pixel, float pixelToPattern)
{
    Disturbance result;
    result.warp = float2(0.0, 0.0);
    result.gradient = float2(0.0, 0.0);
    result.glow = 0.0;

    float radius = max(fieldRadius, 1.0) * pixelToPattern;
    float2 offset = (pixel - fieldPosition) * pixelToPattern;
    float falloff = exp(-dot(offset, offset) / (radius * radius)) * fieldPresence;

    float speed = length(fieldVelocity);
    float2 heading = fieldVelocity / max(speed, 1.0);
    float drag = saturate(speed / (fieldRadius * 20.0));

    float dent = 0.3 + 0.4 * drag;
    result.warp = -heading * drag * 0.4 * falloff;
    result.gradient = -2.0 * dent * offset / (radius * radius) * falloff;
    result.glow = falloff;

    float2 trailOffset = (pixel - fieldTrail) * pixelToPattern;
    float separation = length(fieldTrail - fieldPosition) * pixelToPattern;
    float trailRadius = radius * 0.9;
    float trailFalloff = exp(-dot(trailOffset, trailOffset) / (trailRadius * trailRadius)) * fieldPresence *
                         saturate(separation / (radius * 0.6));
    result.gradient += -2.0 * 0.4 * trailOffset / (trailRadius * trailRadius) * trailFalloff;
    result.glow += 0.3 * trailFalloff;

    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        float4 ripple = fieldRipples[i];
        if (ripple.w <= 0.0)
            continue;

        float2 fromOrigin = (pixel - ripple.xy) * pixelToPattern;
        float distanceToOrigin = length(fromOrigin);
        float width = 0.55 * radius;
        float phase = (distanceToOrigin - 4.0 * radius * ripple.z) / width;
        float envelope = exp(-phase * phase) * ripple.w * 0.11;
        float slope = envelope * (-2.0 * phase * cos(2.4 * phase) - 2.4 * sin(2.4 * phase)) / width;
        result.gradient += slope * fromOrigin / max(distanceToOrigin, 0.0001);
    }
    return result;
}

float4 shadeHero(ShapeInstance shape, float2 fromCenter, float2 halfSize, float2 pixel)
{
    float radius = cornerRadiusAt(fromCenter, shape.radii);
    float coverage = saturate(0.5 - roundedBoxDistance(fromCenter, halfSize, radius));
    float2 local = pixel - shape.rect.xy;
    float pixelToPattern = 0.5 / max(shape.params1.z, 1.0);
    Disturbance disturbance = backdropDisturbance(pixel, pixelToPattern);
    float3 chrome = liquidChrome(local * pixelToPattern + disturbance.warp, shape.params1.x, 0.035, disturbance.gradient) *
                    shape.params1.y * (1.0 + 0.35 * disturbance.glow);
    float vertical = saturate(local.y / max(shape.rect.w, 1.0));
    float fade = lerp(1.0, 1.0 - shape.params1.w, smoothstep(0.0, 0.75, vertical));
    float vignette = saturate(1.0 - 0.45 * dot(fromCenter / halfSize, fromCenter / halfSize) * 0.5);
    float3 rgb = chrome * vignette * fade * shape.fill.rgb;
    return float4(rgb * shape.fill.a, shape.fill.a) * coverage;
}

float4 shadeArt(ShapeInstance shape, float2 fromCenter, float2 halfSize, float2 pixel)
{
    float radius = cornerRadiusAt(fromCenter, shape.radii);
    float coverage = saturate(0.5 - roundedBoxDistance(fromCenter, halfSize, radius));
    float2 uv = (pixel - shape.rect.xy) / shape.rect.zw;
    float hover = shape.params1.y;

    float cardAspect = shape.rect.z / shape.rect.w;
    float imageAspect = 2.0 / 3.0;
    float2 fit = cardAspect > imageAspect ? float2(1.0, imageAspect / cardAspect) : float2(cardAspect / imageAspect, 1.0);
    float zoom = 1.0 + 0.07 * hover;
    float2 sampleUv = (uv - 0.5) * fit / zoom + 0.5;

    float3 rgb = artImages.Sample(glyphSampler, float3(sampleUv, shape.params1.x)).rgb;
    rgb *= 1.0 + 0.1 * hover;

    float bottom = smoothstep(0.42, 1.0, uv.y);
    rgb *= lerp(1.0, 0.12, bottom * shape.params1.z);
    rgb *= 1.0 - shape.params1.w;

    float grain = (hash21(pixel) - 0.5) * 0.02;
    return float4(saturate(rgb + grain), 1.0) * coverage;
}

float4 shadeFlag(ShapeInstance shape, float2 fromCenter, float2 halfSize, float2 pixel)
{
    float radius = cornerRadiusAt(fromCenter, shape.radii);
    float coverage = saturate(0.5 - roundedBoxDistance(fromCenter, halfSize, radius));
    float2 uv = (pixel - shape.rect.xy) / shape.rect.zw;
    float4 sampled = flagImages.Sample(glyphSampler, float3(uv, shape.params1.x));

    float shade = 1.0 - 0.1 * uv.y;
    float3 rgb = saturate(sampled.rgb * shade * shape.params1.y);
    return float4(rgb, 1.0) * coverage;
}

float clipCoverage(ShapeInstance shape, float2 pixel)
{
    float2 halfSize = shape.clip.zw * 0.5;
    float2 fromCenter = pixel - (shape.clip.xy + halfSize);
    return saturate(0.5 - roundedBoxDistance(fromCenter, halfSize, shape.params0.y));
}

float4 psMain(VertexOutput input) : SV_Target
{
    ShapeInstance shape = instances[input.instanceIndex];
    uint kind = (uint)shape.params0.w;

    float2 halfSize = shape.rect.zw * 0.5;
    float2 fromCenter = input.pixel - (shape.rect.xy + halfSize);

    if (kind == kindMask)
        return shadeMask(shape, fromCenter, halfSize);
    if (kind == kindGlass)
        return shadeGlass(shape, input.pixel, fromCenter, halfSize);

    float4 color;
    if (kind == kindShadow)
        color = shadeShadow(shape, fromCenter, halfSize);
    else if (kind == kindGlyph)
        color = shadeGlyph(shape, input.pixel);
    else if (kind == kindHero)
        color = shadeHero(shape, fromCenter, halfSize, input.pixel);
    else if (kind == kindArt)
        color = shadeArt(shape, fromCenter, halfSize, input.pixel);
    else if (kind == kindFlag)
        color = shadeFlag(shape, fromCenter, halfSize, input.pixel);
    else
        color = shadeRect(shape, fromCenter, halfSize);
    return color * (clipCoverage(shape, input.pixel) * shape.params2.x);
}
