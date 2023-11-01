#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormalShadow;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gAoRoughMetal;
layout (location = 4) out vec3 gViewPosition;
layout (location = 5) out vec3 gViewNormal;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 ModelPos;
in vec3 Tangent;
in vec3 Bitangent;
in vec3 Normal;
in vec3 ViewPos;
in vec3 ViewNormal;

in mat3 ViewTBN;

in mat4 TransformedModel;

struct Material {
    vec4 albedo;
    float roughness;
    float metallic;
    // emissive
    vec4 emissiveColor;
    float emissiveStrength;
    // parallax occlusion mapping
    float parallaxMapMidLevel;
    float parallaxMapScale;
    float parallaxMapSampleCount;
    float parallaxMapScaleMode;

    //
    bool albedoMap;
    bool normalMap;
    bool heightMap;
    bool aoMap;
    bool roughMap;
    bool metalMap;
    bool aoRoughMetalMap;
};

uniform Material material;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_metal1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;
uniform sampler2D texture_rough1;
uniform sampler2D texture_ao1;
uniform sampler2D texture_unknown1;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// sun
uniform vec3 lightDirection;

const float PI = 3.14159265359;

// shadow
uniform sampler2DArray ShadowMap;
uniform vec4 FrustumDistances;
uniform vec3 Bias;
uniform vec3 u_camPosition;
uniform float u_shadowFar;

layout (std140) uniform matrices {
    mat4 DepthBiasVP[3];
};

//------------------------------------------------------------------------------
// Shadowing implementation from Filament engine.
// https://github.com/google/filament
// 
// Apache License, Version 2.0

float saturate(float x) {
    return clamp(x, 0.0, 1.0);
}

highp vec4 mulMat4x4Float3(const highp mat4 m, const highp vec3 v) {
    return v.x * m[0] + (v.y * m[1] + (v.z * m[2] + m[3]));
}

mediump vec2 poissonDisk[64] = vec2[]( // don't use 'const' b/c of OSX GL compiler bug
    vec2(0.511749, 0.547686), vec2(0.58929, 0.257224), vec2(0.165018, 0.57663), vec2(0.407692, 0.742285),
    vec2(0.707012, 0.646523), vec2(0.31463, 0.466825), vec2(0.801257, 0.485186), vec2(0.418136, 0.146517),
    vec2(0.579889, 0.0368284), vec2(0.79801, 0.140114), vec2(-0.0413185, 0.371455), vec2(-0.0529108, 0.627352),
    vec2(0.0821375, 0.882071), vec2(0.17308, 0.301207), vec2(-0.120452, 0.867216), vec2(0.371096, 0.916454),
    vec2(-0.178381, 0.146101), vec2(-0.276489, 0.550525), vec2(0.12542, 0.126643), vec2(-0.296654, 0.286879),
    vec2(0.261744, -0.00604975), vec2(-0.213417, 0.715776), vec2(0.425684, -0.153211), vec2(-0.480054, 0.321357),
    vec2(-0.0717878, -0.0250567), vec2(-0.328775, -0.169666), vec2(-0.394923, 0.130802), vec2(-0.553681, -0.176777),
    vec2(-0.722615, 0.120616), vec2(-0.693065, 0.309017), vec2(0.603193, 0.791471), vec2(-0.0754941, -0.297988),
    vec2(0.109303, -0.156472), vec2(0.260605, -0.280111), vec2(0.129731, -0.487954), vec2(-0.537315, 0.520494),
    vec2(-0.42758, 0.800607), vec2(0.77309, -0.0728102), vec2(0.908777, 0.328356), vec2(0.985341, 0.0759158),
    vec2(0.947536, -0.11837), vec2(-0.103315, -0.610747), vec2(0.337171, -0.584), vec2(0.210919, -0.720055),
    vec2(0.41894, -0.36769), vec2(-0.254228, -0.49368), vec2(-0.428562, -0.404037), vec2(-0.831732, -0.189615),
    vec2(-0.922642, 0.0888026), vec2(-0.865914, 0.427795), vec2(0.706117, -0.311662), vec2(0.545465, -0.520942),
    vec2(-0.695738, 0.664492), vec2(0.389421, -0.899007), vec2(0.48842, -0.708054), vec2(0.760298, -0.62735),
    vec2(-0.390788, -0.707388), vec2(-0.591046, -0.686721), vec2(-0.769903, -0.413775), vec2(-0.604457, -0.502571),
    vec2(-0.557234, 0.00451362), vec2(0.147572, -0.924353), vec2(-0.0662488, -0.892081), vec2(0.863832, -0.407206)
);

/*
 * Random number between 0 and 1, using interleaved gradient noise.
 * w must not be normalized (e.g. window coordinates)
 */
float interleavedGradientNoise(highp vec2 w) {
    const vec3 m = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(m.z * fract(dot(w, m.xy)));
}

// tap count up can go up to 64
const uint DPCF_SHADOW_TAP_COUNT                = 12u;
// more samples lead to better "shape" of the hardened shadow
const uint PCSS_SHADOW_BLOCKER_SEARCH_TAP_COUNT = 16u;
// less samples lead to noisier shadows (can be mitigated with TAA)
const uint PCSS_SHADOW_FILTER_TAP_COUNT         = 16u;

//------------------------------------------------------------------------------
// PCF Shadow Sampling
//------------------------------------------------------------------------------

float sampleDepth(const mediump sampler2DArray map,
        const highp vec4 scissorNormalized,
        const uint layer,  highp vec2 uv, float depth) {

    // clamp needed for directional lights and/or large kernels
    uv = clamp(uv, scissorNormalized.xy, scissorNormalized.zw);

    // depth must be clamped to support floating-point depth formats which are always in
    // the range [0, 1].
    // return texture(map, vec4(uv, layer, saturate(depth)));
    return texture(map, vec3(uv, saturate(depth))).r;
}

float hardenedKernel(float x) {
    // this is basically a stronger smoothstep()
    x = 2.0 * x - 1.0;
    float s = sign(x);
    x = 1.0 - s * x;
    x = x * x * x;
    x = s - x * s;
    return 0.5 * x + 0.5;
}

highp vec2 computeReceiverPlaneDepthBias(const highp vec3 position) {
    // see: GDC '06: Shadow Mapping: GPU-based Tips and Techniques
    // Chain rule to compute dz/du and dz/dv
    // |dz/du|   |du/dx du/dy|^-T   |dz/dx|
    // |dz/dv| = |dv/dx dv/dy|    * |dz/dy|
    highp vec3 duvz_dx = dFdx(position);
    highp vec3 duvz_dy = dFdy(position);
    highp vec2 dz_duv = inverse(transpose(mat2(duvz_dx.xy, duvz_dy.xy))) * vec2(duvz_dx.z, duvz_dy.z);
    return dz_duv;
}

mat2 getRandomRotationMatrix(highp vec2 fragCoord) {
    // rotate the poisson disk randomly
    float temporalNoise = 0.0;
    // fragCoord += vec2(frameUniforms.temporalNoise); // 0 when TAA is not used
    fragCoord += vec2(temporalNoise); // 0 when TAA is not used
    float randomAngle = interleavedGradientNoise(fragCoord) * (2.0 * PI);
    vec2 randomBase = vec2(cos(randomAngle), sin(randomAngle));
    mat2 R = mat2(randomBase.x, randomBase.y, -randomBase.y, randomBase.x);
    return R;
}

float getPenumbraLs(const bool DIRECTIONAL, const int index, const highp float zLight) {
    float penumbraScale = 1.0;
    float shadowBulbRadius = 0.02;
    float wsTexelSize = 0.02;
    // s.shadows[shadowIndex].bulbRadiusLs = mSoftShadowOptions.penumbraScale * 
    //                                       options.shadowBulbRadius / wsTexelSize;
    float bulbRadiusLs = penumbraScale * shadowBulbRadius / wsTexelSize;

    float penumbra;
    // This conditional is resolved at compile time
    if (DIRECTIONAL) {
        // penumbra = shadowUniforms.shadows[index].bulbRadiusLs;
        penumbra = bulbRadiusLs;
    } else {
        // the penumbra radius depends on the light-space z for spotlights
        // penumbra = shadowUniforms.shadows[index].bulbRadiusLs / zLight;
        penumbra = bulbRadiusLs / zLight;
    }
    return penumbra;
}

float getPenumbraRatio(const bool DIRECTIONAL, const int index,
        float z_receiver, float z_blocker) {
    // z_receiver/z_blocker are not linear depths (i.e. they're not distances)
    // Penumbra ratio for PCSS is given by:  pr = (d_receiver - d_blocker) / d_blocker
    float penumbraRatio;
    if (DIRECTIONAL) {
        // TODO: take lispsm into account
        // For directional lights, the depths are linear but depend on the position (because of LiSPSM).
        // With:        z_linear = f + z * (n - f)
        // We get:      (r-b)/b ==> (f/(n-f) + r_linear) / (f/(n-f) + b_linear) - 1
        // Assuming f>>n and ignoring LISPSM, we get:
        penumbraRatio = (z_blocker - z_receiver) / (1.0 - z_blocker);
    } else {
        // For spotlights, the depths are congruent to 1/z, specifically:
        //      z_linear = (n * f) / (n + z * (f - n))
        // replacing in (r - b) / b gives:

        // TODO:
        // float nearOverFarMinusNear = shadowUniforms.shadows[index].nearOverFarMinusNear;
        // penumbraRatio = (nearOverFarMinusNear + z_blocker) / (nearOverFarMinusNear + z_receiver) - 1.0;
    }
    // TODO:
    // float shadowPenumbraRatioScale = 1.0;
    // return penumbraRatio * frameUniforms.shadowPenumbraRatioScale;
    return penumbraRatio;
}

void blockerSearchAndFilter(out float occludedCount, out float z_occSum,
        const mediump sampler2DArray map, const highp vec4 scissorNormalized, const highp vec2 uv,
        const float z_rec, const uint layer,
        const highp vec2 filterRadii, const mat2 R, const highp vec2 dz_duv,
        const uint tapCount) {
    occludedCount = 0.0;
    z_occSum = 0.0;
    for (uint i = 0u; i < tapCount; i++) {
        highp vec2 duv = R * (poissonDisk[i] * filterRadii);
        highp vec2 tc = clamp(uv + duv, scissorNormalized.xy, scissorNormalized.zw);

        float z_occ = textureLod(map, vec3(tc, layer), 0.0).r;

        // note: z_occ and z_rec are not necessarily linear here, comparing them is always okay for
        // the regular PCF, but the "distance" is meaningless unless they are actually linear
        // (e.g.: for the directional light).
        // Either way, if we assume that all the samples are close to each other we can take their
        // average regardless, and the average depth value of the occluders
        // becomes: z_occSum / occludedCount.

        // receiver plane depth bias
        float z_bias = dot(dz_duv, duv);
        float dz = z_occ - z_rec; // dz>0 when blocker is between receiver and light
        float occluded = step(z_bias, dz);
        occludedCount += occluded;
        z_occSum += z_occ * occluded;
    }
}

float filterPCSS(const mediump sampler2DArray map,
        const highp vec4 scissorNormalized,
        const highp vec2 size,
        const highp vec2 uv, const float z_rec, const uint layer,
        const highp vec2 filterRadii, const mat2 R, const highp vec2 dz_duv,
        const uint tapCount) {

    float occludedCount = 0.0; // must be highp to workaround a spirv-tools issue
    for (uint i = 0u; i < tapCount; i++) {
        highp vec2 duv = R * (poissonDisk[i] * filterRadii);

        // sample the shadow map with a 2x2 PCF, this helps a lot in low resolution areas
        vec4 d;
        highp vec2 tc = clamp(uv + duv, scissorNormalized.xy, scissorNormalized.zw);
        highp vec2 st = tc.xy * size - 0.5;
        highp vec2 grad = fract(st);

// #if defined(FILAMENT_HAS_FEATURE_TEXTURE_GATHER)
//         d = textureGather(map, vec3(tc, layer), 0); // 01, 11, 10, 00
// #else
        // we must use texelFetchOffset before texelLodOffset filters
        d[0] = texelFetchOffset(map, ivec3(st, layer), 0, ivec2(0, 1)).r;
        d[1] = texelFetchOffset(map, ivec3(st, layer), 0, ivec2(1, 1)).r;
        d[2] = texelFetchOffset(map, ivec3(st, layer), 0, ivec2(1, 0)).r;
        d[3] = texelFetchOffset(map, ivec3(st, layer), 0, ivec2(0, 0)).r;
// #endif

        // receiver plane depth bias
        float z_bias = dot(dz_duv, duv);
        vec4 dz = d - vec4(z_rec); // dz>0 when blocker is between receiver and light
        vec4 pcf = step(z_bias, dz);
        occludedCount += mix(mix(pcf.w, pcf.z, grad.x), mix(pcf.x, pcf.y, grad.x), grad.y);
    }
    return occludedCount * (1.0 / float(tapCount));
}

/*
 * DPCF, PCF with contact hardenning simulation.
 * see "Shadow of Cold War", A scalable approach to shadowing -- by Kevin Myers
 */
float ShadowSample_DPCF(const bool DIRECTIONAL,
        const mediump sampler2DArray map,
        const highp vec4 scissorNormalized,
        const uint layer, const int index,
        const highp vec4 shadowPosition, const highp float zLight) {
    highp vec3 position = shadowPosition.xyz * (1.0 / shadowPosition.w);
    highp vec2 texelSize = vec2(1.0) / vec2(textureSize(map, 0));

    // We need to use the shadow receiver plane depth bias to combat shadow acne due to the
    // large kernel.
    highp vec2 dz_duv = computeReceiverPlaneDepthBias(position);

    float penumbra = getPenumbraLs(DIRECTIONAL, index, zLight);

    // rotate the poisson disk randomly
    mat2 R = getRandomRotationMatrix(gl_FragCoord.xy);

    float occludedCount = 0.0;
    float z_occSum = 0.0;

    blockerSearchAndFilter(occludedCount, z_occSum,
            map, scissorNormalized, position.xy, position.z, layer, texelSize * penumbra, R, dz_duv,
            DPCF_SHADOW_TAP_COUNT);

    // early exit if there is no occluders at all, also avoids a divide-by-zero below.
    if (z_occSum == 0.0) {
        return 1.0;
    }

    float penumbraRatio = getPenumbraRatio(DIRECTIONAL, index, position.z, z_occSum / occludedCount);

    // The main way we're diverging from PCSS is that we're not going to sample again, instead
    // we're going to reuse the blocker search samples and we're going to use the penumbra ratio
    // as a parameter to lerp between a hardened PCF kernel and the search PCF kernel.
    // We need a parameter to blend between the the "hardened" kernel and the "soft" kernel,
    // to this end clamp the penumbra ratio between 0 (blocker is close to the receiver) and
    // 1 (blocker is close to the light).
    penumbraRatio = saturate(penumbraRatio);

    // regular PCF weight (i.e. average of samples in shadow)
    float percentageOccluded = occludedCount * (1.0 / float(DPCF_SHADOW_TAP_COUNT));

    // now we just need to lerp between hardened PCF and regular PCF based on alpha
    percentageOccluded = mix(hardenedKernel(percentageOccluded), percentageOccluded, penumbraRatio);
    return 1.0 - percentageOccluded;
}

float ShadowSample_PCSS(const bool DIRECTIONAL,
        const mediump sampler2DArray map,
        const highp vec4 scissorNormalized,
        const uint layer, const int index,
        const highp vec4 shadowPosition, const highp float zLight) {
    highp vec2 size = vec2(textureSize(map, 0));
    highp vec2 texelSize = vec2(1.0) / size;
    highp vec3 position = shadowPosition.xyz * (1.0 / shadowPosition.w);

    // We need to use the shadow receiver plane depth bias to combat shadow acne due to the
    // large kernel.
    highp vec2 dz_duv = computeReceiverPlaneDepthBias(position);

    float penumbra = getPenumbraLs(DIRECTIONAL, index, zLight);

    // rotate the poisson disk randomly
    mat2 R = getRandomRotationMatrix(gl_FragCoord.xy);

    float occludedCount = 0.0;
    float z_occSum = 0.0;

    blockerSearchAndFilter(occludedCount, z_occSum,
            map, scissorNormalized, position.xy, position.z, layer, texelSize * penumbra, R, dz_duv,
            PCSS_SHADOW_BLOCKER_SEARCH_TAP_COUNT);

    // early exit if there is no occluders at all, also avoids a divide-by-zero below.
    if (z_occSum == 0.0) {
        return 1.0;
    }

    float penumbraRatio = getPenumbraRatio(DIRECTIONAL, index, position.z, z_occSum / occludedCount);

    float percentageOccluded = filterPCSS(map, scissorNormalized, size,
            position.xy, position.z, layer,
            texelSize * (penumbra * penumbraRatio),
            R, dz_duv, PCSS_SHADOW_FILTER_TAP_COUNT);

    return 1.0 - percentageOccluded;
}

//------------------------------------------------------------------------------
// VSM
//------------------------------------------------------------------------------

float linstep(const float min, const float max, const float v) {
    // we could use smoothstep() too
    return clamp((v - min) / (max - min), 0.0, 1.0);
}

float reduceLightBleed(const float pMax, const float amount) {
    // Remove the [0, amount] tail and linearly rescale (amount, 1].
    return linstep(amount, 1.0, pMax);
}

float chebyshevUpperBound(const highp vec2 moments, const highp float mean,
        const highp float minVariance, const float lightBleedReduction) {
    // Donnelly and Lauritzen 2006, "Variance Shadow Maps"

    highp float variance = moments.y - (moments.x * moments.x);
    variance = max(variance, minVariance);

    highp float d = mean - moments.x;
    float pMax = variance / (variance + d * d);

    pMax = reduceLightBleed(pMax, lightBleedReduction);

    return mean <= moments.x ? 1.0 : pMax;
}

float evaluateShadowVSM(const highp vec2 moments, const highp float depth) {
    // frameUniforms
    highp float vsmDepthScale = 1.0;
    highp float vsmLightBleedReduction = 1.0;

    highp float depthScale = vsmDepthScale * depth;
    highp float minVariance = depthScale * depthScale;
    return chebyshevUpperBound(moments, depth, minVariance, vsmLightBleedReduction);
}

float ShadowSample_VSM(const bool ELVSM, const highp sampler2DArray shadowMap,
        const highp vec4 scissorNormalized,
        const uint layer, const highp vec4 shadowPosition) {

    // note: shadowPosition.z is in linear light-space normalized to [0, 1]
    //  see: ShadowMap::computeVsmLightSpaceMatrix() in ShadowMap.cpp
    //  see: computeLightSpacePosition() in common_shadowing.fs
    highp vec3 position = vec3(shadowPosition.xy * (1.0 / shadowPosition.w), shadowPosition.z);

    // Note: we don't need to clamp to `scissorNormalized` in the VSM case because this is only
    // needed when the shadow casters and receivers are different, which is never the case with VSM
    // (see ShadowMap.cpp).

    // Read the shadow map with all available filtering
    highp vec4 moments = texture(shadowMap, vec3(position.xy, layer));
    highp float depth = position.z;
    
    // frameUniforms
    float vsmExponent = 1.0;
    // EVSM depth warping
    depth = depth * 2.0 - 1.0;
    depth = vsmExponent * depth;

    depth = exp(depth);
    float p = evaluateShadowVSM(moments.xy, depth);
    if (ELVSM) {
        p = min(p, evaluateShadowVSM(moments.zw, -1.0 / depth));
    }
    return p;
}

/**
 * Computes the light space position of the specified world space point.
 * The returned point may contain a bias to attempt to eliminate common
 * shadowing artifacts such as "acne". To achieve this, the world space
 * normal at the point must also be passed to this function.
 * Normal bias is not used for VSM.
 */

highp vec4 computeLightSpacePosition(highp vec3 p, const highp vec3 n,
        const highp vec3 dir, const float b, mat4 lightFromWorldMatrix) {

// #if !defined(VARIANT_HAS_VSM)
    highp float cosTheta = saturate(dot(n, dir));
    highp float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    p += n * (sinTheta * b);
// #endif

    return mulMat4x4Float3(lightFromWorldMatrix, p);
}

//------------------------------------------------------------------------------

float getVisibility(vec3 worldPos, vec3 normal) {
    int index = 2;
    if (gl_FragCoord.z < 0) {
    } else if (gl_FragCoord.z < FrustumDistances.x) {
        index = 0;
    } else if (gl_FragCoord.z < FrustumDistances.y) {
        index = 1;
    } else if (gl_FragCoord.z < FrustumDistances.z) {
        index = 2;
    }

    // TODO: ?
    float fragToLight = saturate(dot(normal, lightDirection));
    if (fragToLight == 0.0) {
        return 1.0;
    }

    float bias = Bias[index];
    mat4 DepthBiasMVP = DepthBiasVP[index];
    // vec4 ShadowCoord = DepthBiasMVP * vec4(ModelPos, 1);
    highp vec4 ShadowCoord = computeLightSpacePosition(
            worldPos, normal,
            lightDirection,
            bias,
            DepthBiasMVP);

    highp vec4 scissorNormalized = vec4(0.0, 0.0, 1.0, 1.0);
    int layer = index;
    float zLight = 0.0;
    float shadow = 0.0;

    shadow = ShadowSample_DPCF(
        true,
        ShadowMap,
        scissorNormalized,
        layer,
        index,
        ShadowCoord,
        zLight
    );

    // shadow = ShadowSample_PCSS(
    //     true,
    //     ShadowMap,
    //     scissorNormalized,
    //     layer,
    //     index,
    //     ShadowCoord,
    //     zLight
    // );

    // bool elvsm = true; 
    // shadow = ShadowSample_VSM(
    //     elvsm,
    //     ShadowMap,
    //     scissorNormalized,
    //     layer,
    //     ShadowCoord
    // );

    // TODO: ?
    float visibility = 1.0 - shadow;
    // shadow far attenuation
    highp vec3 v = worldPos - u_camPosition;
    // (viewFromWorld * v).z == dot(transpose(viewFromWorld), v)
    highp float z = dot(transpose(view)[2].xyz, v);
    // highp vec2 p = frameUniforms.shadowFarAttenuationParams;
    highp vec2 p = u_shadowFar > 0.0 ? 0.5 * vec2(10.0, 10.0 / (u_shadowFar * u_shadowFar)) : vec2(1.0, 0.0);
    visibility = 1.0 - ((1.0 - visibility) * saturate(p.x - z * z * p.y));

    return visibility;
}

// ----------------------------------------------------------------------------
// Parallax Occlusion Mapping implementation by Michael Möller for Blender
// https://devtalk.blender.org/t/parallax-occlusion-mapping/15774
// https://archive.blender.org/developer/D9198
// https://archive.blender.org/developer/D9792
// Apache License, Version 2.0

// http://burtleburtle.net/bob/c/lookup3.c
// GPL-2.0-or-later

#define rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

#define final(a, b, c) \
  { \
    c ^= b; \
    c -= rot(b, 14); \
    a ^= c; \
    a -= rot(c, 11); \
    b ^= a; \
    b -= rot(a, 25); \
    c ^= b; \
    c -= rot(b, 16); \
    a ^= c; \
    a -= rot(c, 4); \
    b ^= a; \
    b -= rot(a, 14); \
    c ^= b; \
    c -= rot(b, 24); \
  }

uint hash_uint2(uint kx, uint ky)
{
  uint a, b, c;
  a = b = c = 0xdeadbeefu + (2u << 2u) + 13u;

  b += ky;
  a += kx;
  final(a, b, c);

  return c;
}

float hash_uint2_to_float(uint kx, uint ky)
{
  return float(hash_uint2(kx, ky)) / float(0xFFFFFFFFu);
}

float hash_vec2_to_float(vec2 k)
{
  return hash_uint2_to_float(floatBitsToUint(k.x), floatBitsToUint(k.y));
}

float safe_inverse(float x)
{
  return (x != 0.0) ? 1.0 / x : 0.0;
}

void node_parallax_occlusion_map(vec3 co,
                                 float midlevel,
                                 float scale,
                                 float samples_count,
                                 float scale_mode,
                                 vec3 viewPosition,
                                 vec3 viewNormal,
                                 sampler2D heightmap,
                                 out vec3 out_co,
                                 out float depth,
                                 out vec3 normal,
                                 out vec3 out_worldposition)
{
  vec2 dcodx = dFdx(co.xy);
  vec2 dcody = dFdy(co.xy);

  vec3 dPdx = dFdx(viewPosition);
  vec3 dPdy = dFdy(viewPosition);

  mat3 ViewMatrixInverse = mat3(inverse(view));
  vec3 dWdx = mat3(ViewMatrixInverse) * dPdx;
  vec3 dWdy = mat3(ViewMatrixInverse) * dPdy;

  vec3 I = (projection[3][3] == 0.0) ? viewPosition : vec3(0.0, 0.0, -1.0);
  vec3 N = normalize(viewNormal);

  float NdotI = dot(N, I);

  vec3 v1 = cross(dPdy, N);
  vec3 v2 = cross(N, dPdx);
  float det = dot(dPdx, v1);
  float f = safe_inverse(dcodx.x * dcody.y - dcodx.y * dcody.x);

  if (scale_mode > 0) {
    scale *= sqrt(abs(det * f));
  }

  float scale_per_sample = -scale / samples_count;
 
  vec2 s = safe_inverse(det) * vec2(dot(v1, I), dot(v2, I));
  vec2 ss = (safe_inverse(det * NdotI) * scale_per_sample) * vec2(dot(v1, I), dot(v2, I));
  vec2 d = -safe_inverse(NdotI) * scale * (dcodx * s.x + dcody * s.y);
  float r = hash_vec2_to_float(co.xy) - 0.5;
  float e = (midlevel - 1.0) * samples_count + r;

  float inverse_samples_count = 1.0 / samples_count;
  vec3 step = inverse_samples_count * vec3(d, -1.0);
  vec3 uvw = vec3(co.xy - (1 - midlevel) * d, 1.0) + r * step;

  ivec2 heightmap_size = textureSize(heightmap, 0);
  vec2 dtcdx = dcodx * heightmap_size;
  vec2 dtcdy = dcody * heightmap_size;
  float lod = max(0.0, 0.5 * log2(min(dot(dtcdx, dtcdx), dot(dtcdy, dtcdy))));

  vec3 step_W = dWdx * ss.x + dWdy * ss.y;
  vec3 W = WorldPos + e * step_W;

  /* linear search */
  for (float i = 0.0; i < samples_count; i++) {
    vec3 next = uvw + step;
    vec3 next_W = W + step_W;

    float h = textureLod(heightmap, next.xy, lod).x;
    if (h < next.z) {
      uvw = next;
      W = next_W;
    }
    else {
      break;
    }
  }

  /* binary search */
  for (int i = 0; i < 8; i++) {
    step *= 0.5;
    step_W *= 0.5;
    vec3 next = uvw + step;
    vec3 next_W = W + step_W;
    float h = textureLod(heightmap, next.xy, lod).x;
    if (h < next.z) {
      uvw = next;
      W = next_W;
    }
  }

  out_co = uvw;
  out_worldposition = W;
  vec2 offset = uvw.xy - co.xy;

  float vx = f * (dcody.y * offset.x - dcody.x * offset.y);
  float vy = f * (-dcodx.y * offset.x + dcodx.x * offset.y);
  float vz = scale * (uvw.z - midlevel);

  float Xz = dPdx.z - dot(dPdx, N) * N.z;
  float Yz = dPdy.z - dot(dPdy, N) * N.z;
  depth = -(vx * Xz + vy * Yz + vz * N.z);

  float radius = 1.0 / max(heightmap_size.x, heightmap_size.y);
  float dHdu = textureLod(heightmap, uvw.xy + vec2(radius, 0.0), lod).x -
               textureLod(heightmap, uvw.xy - vec2(radius, 0.0), lod).x;
  float dHdv = textureLod(heightmap, uvw.xy + vec2(0.0, radius), lod).x -
               textureLod(heightmap, uvw.xy - vec2(0.0, radius), lod).x;

  vec3 dPdu = f * (dcody.y * dPdx - dcodx.y * dPdy);
  vec3 dPdv = f * (dcodx.x * dPdy - dcody.x * dPdx);
  vec3 r1 = cross(dPdv, N);
  vec3 r2 = cross(N, dPdu);

  float dr = dot(dPdu, r1) * radius * 2.0;
  vec3 surfgrad = dHdu * r1 + dHdv * r2;
  normal = mat3(inverse(view)) * normalize(abs(dr) * N - (sign(dr) * scale) * surfgrad);

  if (projection[3][3] == 0.0 && NdotI > -vz)
    discard;
}

// ----------------------------------------------------------------------------

void main()
{
    vec2 pTexCoords = TexCoords;
    vec3 pNormal = Normal;
    vec3 pWorldPos = WorldPos;

    if (material.heightMap) {
        vec3 out_co;
        float depth_offset;

        // TODO: variable
        node_parallax_occlusion_map(
            vec3(TexCoords, 0.0),
            material.parallaxMapMidLevel,
            material.parallaxMapScale,
            material.parallaxMapSampleCount,
            material.parallaxMapScaleMode,
            ViewPos,
            ViewNormal,
            texture_height1,
            out_co,                 // out vec3 out_co,
            depth_offset,           // out float depth,
            pNormal,                // out vec3 normal
            pWorldPos               // out vec3 out_worldposition
        );

        pTexCoords = out_co.xy;
        
        // TODO: disable option
        // TODO: use depth_offset
        vec4 v_clip_coord = (projection * view * vec4(pWorldPos, 1));
        float f_ndc_depth = v_clip_coord.z / v_clip_coord.w;
        gl_FragDepth = (1.0 - 0.0) * 0.5 * f_ndc_depth + (1.0 + 0.0) * 0.5;
    } else {
        // TODO: when exactly should set?
        gl_FragDepth = gl_FragCoord.z;
    }

    // TODO: preprocessor texture read - material properties
    vec3 albedo;
    float ao, roughness, metallic;

    if (material.albedoMap) {
        albedo = texture(texture_diffuse1, pTexCoords).rgb;
        // TODO: validate
        albedo *= material.albedo.rgb;
    } else {
        albedo = material.albedo.rgb;
    }

    vec3 emissive = material.emissiveColor.xyz * material.emissiveStrength;
    albedo += emissive;

    // TODO: merge detection
    if (material.aoRoughMetalMap) {
        // TODO: individual present
        vec3 merged = texture(texture_unknown1, pTexCoords).rgb;
        ao = 1.0; // TODO: useAOMAP
        roughness = merged.g;
        metallic = merged.b;
    } else {
        if (material.aoMap) {
            ao = texture(texture_ao1, pTexCoords).r;
        } else {
            ao = 1.0;
        }
        if (material.roughMap) {
            roughness = texture(texture_rough1, pTexCoords).r;
        } else {
            roughness = material.roughness;
        }
        if (material.metalMap) {
            metallic = texture(texture_metal1, pTexCoords).r;
        } else {
            metallic = material.metallic;
        }
    }

    // TODO: pom view space position and normal?
    vec3 N;
    vec3 ViewN;
    if (material.normalMap) {
        vec3 tangentNormal = texture(texture_normal1, pTexCoords).xyz * 2.0 - 1.0;
        // TODO: pom tangent and bitangent?
        mat3 TBN = mat3(
            Tangent,
            Bitangent,
            pNormal
        );
        N = normalize(TBN * tangentNormal);
        ViewN = normalize(ViewTBN * tangentNormal);
    } else {
        N = normalize(pNormal);
        ViewN = normalize(ViewNormal);
    }

    gPosition = pWorldPos;
    gNormalShadow = vec4(N, getVisibility(pWorldPos, pNormal));
    gAlbedo = albedo;
    gAoRoughMetal.r = ao;
    gAoRoughMetal.g = roughness;
    gAoRoughMetal.b = metallic;
    gViewPosition = ViewPos;
    gViewNormal = ViewN;

    // TODO: debug frustumIndex
    // if (frustumIndex == 0) {
    //     albedo = vec3(1, 0, 0);
    // } else if (frustumIndex == 1) {
    //     albedo = vec3(1, 1, 0);
    // } else if (frustumIndex == 2) {
    //     albedo = vec3(1, 0, 1);
    // }

    // gAlbedo = vec3(0, 0, 0);
    // gAlbedo = WorldPos;
    // gAlbedo = vec3(albedo);
    // gAlbedo = vec3(metallic);
    // gAlbedo = vec3(roughness);
    // gAlbedo = vec3(ao);
    // gAlbedo = N;
    // gAlbedo = irradiance;
    // gAlbedo = texture(texture_diffuse1, pTexCoords).rgb;
    // gAlbedo = texture(texture_normal1, pTexCoords).rgb;
    // gAlbedo = Normal;
    // gAlbedo = texture(texture_metal1, pTexCoords).rgb;
    // gAlbedo = texture(texture_height1, pTexCoords).rgb;
    // gAlbedo = texture(texture_rough1, pTexCoords).rgb;
    // gAlbedo = texture(texture_ao1, pTexCoords).rgb;
    // gAlbedo = texture(texture_unknown1, pTexCoords).ggg;
    // gAlbedo = texture(irradianceMap, N).rgb;
    // gAlbedo = textureLod(prefilterMap, N, 0).rgb;
    // gAlbedo = texture(brdfLUT, N.xy).rgb;
}