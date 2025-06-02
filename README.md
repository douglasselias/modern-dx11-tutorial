# A modern Direct3D 11 tutorial

Note exactly a tutorial, but more of a reference guide to learn the API.

Beware that I'm not an expert in DX 11. I made this "tutorial" because others use outdated libs, use too much modern C++ features and OOP. All this makes it harder to understand what's going on.

Each example is self contained and aims to be an increment of the last one. I tried to add as little as possible to each step because most tutorials on DX 11 are too bloated with unnecessary code that distracts from learning the API. For example, the third code example is under 200 lines of code, both the C++ and HLSL combined.

Each example has a `.cpp` file for the code, a `.hlsl` file for the Vertex and Pixel shader and a `.md` with some step by step explanation of the code. I don't try to explain math, but you can read more about on [Learn OpenGL site.](https://learnopengl.com/)

Also, I don't explain the API in detail, most of it is self explanatory and the rest is recommended to read the official docs. Other things I really don't know why the API works that way.

I would love to make more advanced examples to share, but I'm still learning. If you happen to be an expert, feel free to add more in-depth explanations, both for math and API, and add more advanced examples, like MSAA, Compute Shader, Shadows, or anything that would help people to create an awesome 3D engine. Maybe even a complete 3D platfomer game with physics and mesh skinning animation!