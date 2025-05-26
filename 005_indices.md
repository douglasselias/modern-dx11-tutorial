## Part 5 - Indices

This requires a few changes only. First, add a new point for our vertices, to make a rectangle.

```cpp
f32 my_vertices[] =
{
  -0.5,  0.75, 1, 1, 1, 0.0, 1.0, // Move the X coordinate to the left instead of center. Same for texture coordinates.
  -0.5, -0.75, 1, 1, 1, 0.0, 0.0,
   0.5, -0.75, 1, 1, 1, 1.0, 0.0,

   0.5,  0.75, 1, 1, 1, 1.0, 1.0, // New top right vertex.
};
```

Now we do something similar to the vertices buffer.

```cpp
u32 my_indices[] =
{
  0, 1, 2,
  0, 2, 3,
};

u32 my_indices_size = ARRAYSIZE(my_indices);

ID3D11Buffer* my_indices_buffer;
{
  D3D11_BUFFER_DESC buffer_desc = {};
  buffer_desc.ByteWidth = sizeof(u32) * my_indices_size;
  buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
  buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  
  D3D11_SUBRESOURCE_DATA subresource = { my_indices };
  
  device->CreateBuffer(&buffer_desc, &subresource, &my_indices_buffer);
}
```

In the main loop we set the indexes. Then use `DrawIndexed` function instead of `Draw`.

````cpp
device_context->IASetIndexBuffer(my_indices_buffer, DXGI_FORMAT_R32_UINT, 0);

// ...

device_context->DrawIndexed(my_indices_size, 0, 0);
```

The shader remains unchanged.