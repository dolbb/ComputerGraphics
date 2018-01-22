#version 330

in vec3 vPosition;
in vec3 vNormal;

out Vertex
{
  vec3 normal;
  vec4 color;
} vertex;

void main()
{
  gl_Position = vec4(vPosition,1.0);
  vertex.normal = vNormal;
  vertex.color =  vec4(1.0, 1.0, 0.0, 1.0);
}