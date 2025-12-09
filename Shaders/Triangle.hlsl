// 定义顶点着色器输出 / 片元着色器输入结构
struct VSOutput
{
    float4 Pos : SV_POSITION; // SV_POSITION 对应 Vulkan 的 gl_Position
    float3 Color : COLOR0; // 传递颜色给 PS
};

// -----------------------------------------------------------
// 顶点着色器 (Vertex Shader)
// 入口函数名: VSMain
// -----------------------------------------------------------
VSOutput VSMain(uint VertexID : SV_VertexID) // SV_VertexID 对应 gl_VertexIndex
{
    VSOutput output;

    // 硬编码三角形坐标 (Vulkan 坐标系: Y向下)
    // 0: 顶部中心 (0.0, -0.5)
    // 1: 右下 (0.5, 0.5)
    // 2: 左下 (-0.5, 0.5)
    float2 positions[3] =
    {
        float2(0.0, -0.5),
        float2(0.5, 0.5),
        float2(-0.5, 0.5)
    };

    // 硬编码颜色
    float3 colors[3] =
    {
        float3(1.0, 0.0, 0.0), // 红
        float3(0.0, 1.0, 0.0), // 绿
        float3(0.0, 0.0, 1.0) // 蓝
    };

    // 输出位置 (z=0.0, w=1.0)
    output.Pos = float4(positions[VertexID], 0.0, 1.0);
    output.Color = colors[VertexID];

    return output;
}

// -----------------------------------------------------------
// 片元着色器 (Pixel Shader)
// 入口函数名: PSMain
// -----------------------------------------------------------
float4 PSMain(VSOutput input) : SV_TARGET // SV_TARGET 对应 layout(location=0) out
{
    // 直接输出插值后的颜色
    return float4(input.Color, 1.0);
}