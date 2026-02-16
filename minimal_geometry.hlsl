
struct GS_OUTPUT
{
    float4 Position : SV_Position;
};
[maxvertexcount(3)]
void gs_main(point Vertex IN[1], inout TriangleStream<GS_OUTPUT> triStream)
{
  GS_OUTPUT OUT = (GS_OUTPUT)0;
  for (int i = 0; i < 3; i++)
  {
    OUT.Position.x++;
    triStream.Append(OUT);
  }
}