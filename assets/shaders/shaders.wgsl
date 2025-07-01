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

struct Camera
{
    view: mat4x4f,
    project: mat4x4f,
    viewproject: mat4x4f,
}

struct ObjectTransform
{
    modelTransform: mat4x4f,
    normalTransform: mat4x4f,
}

// Scene data bind group
@group(0) @binding(0) var<uniform> camera: Camera;

// Object data bind group
@group(1) @binding(0) var<uniform> objectTransform: ObjectTransform;

@vertex
fn VSStaticVert(input: VertexInput) -> VertexOutput
{
    // Transform object position & vectors
    let position = objectTransform.modelTransform * vec4f(input.position, 1.);
    let normal = objectTransform.normalTransform * vec4f(input.normal, 0);
    let tangent = objectTransform.normalTransform * vec4f(input.tangent, 0);

    // Calculate normal, tangent, bitangent
    let N = normalize(normal.xyz);
    var T = normalize(tangent.xyz);
    T = normalize(T - dot(T, N) * N); // Reorthogonalize normal and tangent
    let B = cross(N, T);

    var result = VertexOutput();
    result.position = camera.viewproject * position;
    result.normal = N;
    result.tangent = T;
    result.bitangent = B;
    result.texcoord = input.texcoord;

    return result;
}

@fragment
fn FSForwardShading(input: VertexOutput) -> @location(0) vec4f
{
    let color = vec4f(input.texcoord, 0, 1);
    return color;
}
