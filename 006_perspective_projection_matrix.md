## Part 6 - Perspective projection matrix

First we need to define the transform value of the object to display. For this we use the constant buffer, since the transform matrix is the same for every vertex.

```hlsl
cbuffer constants : register(b0)
{
  row_major float4x4 transform;
}

// ...

output.position = mul(float4(v.position, -45, 1), transform);
```

Now we create the constant buffer.

```cpp
struct Matrix { f32 m[4][4]; };
struct Constants { Matrix transform; };

// ...

ID3D11Buffer* constants_buffer;
{
  D3D11_BUFFER_DESC buffer_desc = {};
  buffer_desc.ByteWidth      = sizeof(Constants);
  buffer_desc.Usage          = D3D11_USAGE_DYNAMIC;
  buffer_desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      
  device->CreateBuffer(&buffer_desc, null, &constants_buffer);
}
```

For convinience we create a operator overload for matrix multiplication.

```cpp
Matrix operator*(const Matrix& m1, const Matrix& m2)
{
  return
  {
    m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0],
    m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1],
    m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2],
    m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3],
    m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0],
    m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1],
    m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2],
    m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3],
    m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0],
    m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1],
    m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2],
    m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3],
    m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0],
    m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1],
    m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2],
    m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3],
  };
}
```

We will also need to have a projection matrix.
https://jsantell.com/3d-projection/

```cpp
#undef near
#undef far

Matrix create_projection_matrix(f32 aspect, f32 fov, f32 far, f32 near)
{
  Matrix result = {};

  f32 focal_length = 1 / (tanf(fov / 2));

  result.m[0][0] = focal_length / aspect;
  result.m[1][1] = focal_length;
  result.m[2][2] = (far + near) / (near - far);

  result.m[2][3] = (2 * far * near) / (near - far);

  result.m[3][2] = -1;

  return result;
}
```

Also, create a rotation matrix to be able to rotate the texture in the Z axis.

```cpp
Matrix create_z_axis_rotation_matrix(f32 angle)
{
  f32 sine   = sinf(angle);
  f32 cosine = cosf(angle);

  return
  {
    cosine, -sine,   0, 0,
    sine,    cosine, 0, 0,
    0,       0,      1, 0,
    0,       0,      0, 1,
  };
}
```

Now we setup the projection matrix.

```cpp
f32 aspect = viewport.Width / viewport.Height;
f32 fov = 90.0f * (3.14159f / 180.0f);
f32 near = 0.01f;
f32 far = 100.0f;
Matrix projection = create_projection_matrix(aspect, fov, far, near);

struct V3 { f32 x, y, z; };

V3 my_rotation = {};
```

Finally, in the main loop, we update the angle in the Z axis rotation and map the data into the shader.

```cpp
my_rotation.z += 0.009f;

Matrix rotation_z = create_z_axis_rotation_matrix(my_rotation.z);
Matrix transform = rotation_z * projection;

// ...

D3D11_MAPPED_SUBRESOURCE constants_subresource;
device_context->Map(constants_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constants_subresource);
{
  Constants* constants = (Constants*)constants_subresource.pData;
  constants->transform = transform;
}
device_context->Unmap(constants_buffer, 0);

device_context->VSSetConstantBuffers(0, 1, &constants_buffer);
```