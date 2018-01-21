#version 330

in vec3 vPosition;
in vec3 vNormal;

out Vertex
{
  vec4 normal;
  vec4 color;
} vertex;

void main()
{
  gl_Position = vPosition;
  vertex.normal = vNormal;
  vertex.color =  vec4(1.0, 1.0, 0.0, 1.0);
}