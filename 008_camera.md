## Part 8 - Camera

Add some vector math necessary for moving the camera.

```cpp

f32 dot(V3 v1, V3 v2)
{
  V3 v = v1 * v2;
  f32 result = v.x + v.y + v.z;
  return result;
}

V3 normalize(V3 v)
{
  f32 length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

  if(length != 0.0f)
  {
    f32 inverse_length = 1.0f / length;
    v.x *= inverse_length;
    v.y *= inverse_length;
    v.z *= inverse_length;
  }

  return v;
}

V3 cross(V3 v1, V3 v2)
{
  V3 result =
  {
    v1.y * v2.z - v1.z * v2.y,
    v1.z * v2.x - v1.x * v2.z,
    v1.x * v2.y - v1.y * v2.x,
  };

  return result;
}

V3 v3_rotate_by_axis_angle(V3 x, V3 axis, f32 angle)
{
  angle /= 2.0f;

  V3 w   = normalize(axis) * sinf(angle);
  V3 wv  = cross(w, x)     * (cosf(angle) * 2);
  V3 wwv = cross(w, wv)    * 2;

  V3 result = x + wv + wwv;

  return result;
}
```

Create functions to rotate the camera.

```cpp
void camera_yaw(Camera* camera, f32 angle)
{
  V3 target_position = camera->target - camera->position;

  target_position = v3_rotate_by_axis_angle(target_position, up_base, angle);

  camera->target = camera->position + target_position;
}

void camera_pitch(Camera* camera, f32 angle)
{
  V3 target_position = camera->target - camera->position;
  V3 forward = normalize(target_position);
  V3 right = normalize(cross(up_base, forward));

  target_position = v3_rotate_by_axis_angle(target_position, right, angle);
  camera->target = camera->position + target_position;
}
```

Create a function to respond to keyboard inputs to move the camera.

```cpp
void update_camera(Camera* camera, f32 dt)
{
  V3 forward = normalize(camera->target - camera->position);
  V3 right   = normalize(cross(forward, up_base));

  f32 movement_speed = 4.75f * dt;

  // Move forward
  if(GetAsyncKeyState('W') & 0x8000)
  {
    V3 movement = forward * -movement_speed;
    camera->position = camera->position + movement;
    camera->target   = camera->target   + movement;
  }

  // Move backward
  if(GetAsyncKeyState('S') & 0x8000)
  {
    V3 movement = forward * movement_speed;
    camera->position = camera->position + movement;
    camera->target   = camera->target   + movement;
  }

  // Strafe left
  if(GetAsyncKeyState('A') & 0x8000)
  {
    V3 movement = right * -movement_speed;
    camera->position = camera->position + movement;
    camera->target   = camera->target   + movement;
  }
  
  // Strafe right
  if (GetAsyncKeyState('D') & 0x8000)
  {
    V3 movement = right * movement_speed;
    camera->position = camera->position + movement;
    camera->target   = camera->target   + movement;
  }

  // Move up
  if (GetAsyncKeyState('Q') & 0x8000) 
  {
    V3 movement = up_base * movement_speed;
    camera->position = camera->position + movement;
    camera->target   = camera->target   + movement;
  }
  
  // Move down
  if(GetAsyncKeyState('E') & 0x8000)
  {
    V3 movement = up_base * -movement_speed;
    camera->position = camera->position + movement;
    camera->target   = camera->target   + movement;
  }

  f32 rotation_speed = 1;
  f32 rotation_amount = rotation_speed * dt;

  // Look left
  if(GetAsyncKeyState('J') & 0x8000)
  {
    camera_yaw(camera, -rotation_amount);
  }

  // Look right
  if(GetAsyncKeyState('L') & 0x8000)
  {
    camera_yaw(camera, rotation_amount);
  }

  // Look down
  if(GetAsyncKeyState('I') & 0x8000)
  {
    camera_pitch(camera, rotation_amount);
  }

  // Look up
  if(GetAsyncKeyState('K') & 0x8000)
  {
    camera_pitch(camera, -rotation_amount);
  }
}
```

Then create the projection matrix.

```cpp
Matrix create_projection_matrix(f32 aspect, f32 near, f32 far)
{
  return
  {
    2 * near / aspect, 0,        0,                         0,
    0,                 2 * near, 0,                         0,
    0,                 0,        far / (far - near),        1,
    0,                 0,        near * far / (near - far), 0,
  };
}
```

Change the sampler to `sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;`

Update the constant buffer description to align the byte width to 16 bit. This is because DX11 expects this.

`buffer_desc.ByteWidth = sizeof(Constants) + 0xf & 0xfffffff0;`

Add the projection matrix in the transform matrix.

`Matrix transform =  rotation_x * rotation_y * rotation_z * my_translation_matrix * view * projection;`