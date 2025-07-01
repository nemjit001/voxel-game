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
    @location(0) texcoord: vec2f,
}

@vertex
fn VSStaticVert(input: VertexInput) -> VertexOutput
{
    var result = VertexOutput();
    result.position = vec4f(input.position, 1.);
    result.texcoord = input.texcoord;

    return result;
}

@fragment
fn FSForwardShading(input: VertexOutput) -> @location(0) vec4f
{
    let color = vec4f(input.texcoord, 0, 1);
    return color;
}
