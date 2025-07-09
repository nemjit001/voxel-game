struct VertexInput
{
    @location(0) position: vec3f,
    @location(1) normal: vec3f,
    @location(2) tangent: vec3f,
    @location(3) texcoord: vec2f,
}

struct VertexOutput
{
    @builtin(position) position: vec4f,
    @location(0) normal: vec3f,
    @location(1) tangent: vec3f,
    @location(2) bitangent: vec3f,
    @location(3) texcoord: vec2f,
}

struct FragmentOutput
{
    @location(0) color: vec4f,
}

struct Camera
{
    view: mat4x4f,
    project: mat4x4f,
    viewproject: mat4x4f,
}

struct Material
{
    hasAlbedoMap: u32,
    hasNormalMap: u32,
}

struct ObjectTransform
{
    modelTransform: mat4x4f,
    normalTransform: mat4x4f,
}

// Scene data bind group
@group(0) @binding(0) var<uniform> camera: Camera;

// Material data bind group
@group(1) @binding(0) var<uniform> material: Material;
@group(1) @binding(1) var linearSampler: sampler;
@group(1) @binding(2) var albedoMap: texture_2d<f32>;
@group(1) @binding(3) var normalMap: texture_2d<f32>;

// Object data bind group
@group(2) @binding(0) var<uniform> objectTransform: ObjectTransform;

@vertex
fn VSStaticVert(input: VertexInput) -> VertexOutput
{
    // Transform object position & vectors
    let position = objectTransform.modelTransform * vec4f(input.position, 1.);
    let normal = objectTransform.normalTransform * vec4f(input.normal, 0);
    let tangent = objectTransform.normalTransform * vec4f(input.tangent, 0);

    // Calculate normal, tangent, bitangent
    var N = normalize(normal.xyz);
    var T = normalize(tangent.xyz);
    T = normalize(T - dot(T, N) * N); // Reorthogonalize normal and tangent
    var B = cross(N, T);

    var result = VertexOutput();
    result.position = camera.viewproject * position;
    result.normal = N;
    result.tangent = T;
    result.bitangent = B;
    result.texcoord = input.texcoord;

    return result;
}

@fragment
fn FSForwardShading(input: VertexOutput) -> FragmentOutput
{
    // Set up TBN matrix
    let TBN = mat3x3f(input.tangent, input.bitangent, input.normal);

    // Get albedo color
    var albedo = vec4f(0, 0, 0, 1);
    if (material.hasAlbedoMap != 0)
    {
        albedo = textureSample(albedoMap, linearSampler, input.texcoord);
    }

    // Get shading normal
    var normal = vec3f(0, 0, 1);
    if (material.hasNormalMap != 0)
    {
        normal = textureSample(normalMap, linearSampler, input.texcoord).xyz;
        normal = 2.0 * normal - 1.0; // Remap normal to range [-1, 1]
    }

    // TODO(nemjit001): do some shading based on light positions in scene
    let shadingNormal = normalize(TBN * normal);

    var result = FragmentOutput();
    result.color = albedo;

    return result;
}
