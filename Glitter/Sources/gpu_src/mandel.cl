const float2 xMinMax = (float2)(-2.0f, 0.47f);
const float2 yMinMax = (float2)(-1.12f, 1.12f);
const float scale = 1.0f;
const int maxIter = 1000;

float4 lerp(float4 a, float4 b, float t)
{
	return (1.0f - t) * a + t * b;
}

kernel void test(__global int* test_buf)
{
	int x = get_global_id(0);
	//int y = get_global_id(1);

	test_buf[x] = x;
}

//x0: = scaled x coordinate of pixel(scaled to lie in the Mandelbrot X scale(-2.00, 0.47))
//y0 : = scaled y coordinate of pixel(scaled to lie in the Mandelbrot Y scale(-1.12, 1.12))
//x : = 0.0
//y : = 0.0
//iteration : = 0
//max_iteration : = 1000
//while (x * x + y * y ≤ 2 * 2 AND iteration < max_iteration) do
//    xtemp : = x * x - y * y + x0
//    y : = 2 * x * y + y0
//    x : = xtemp
//    iteration : = iteration + 1
//
//    color : = palette[iteration]
//    plot(Px, Py, color)
kernel void Mandel(write_only image2d_t res, float dx, float dy, float scale)
{
	// x0{ ((xMax - xMin) * va[i].position.x / width + xMin) / scale + dx };
	// y0{ ((yMax - yMin) * (height - va[i].position.y) / height + yMin) / scale + dy };

	const int x = get_global_id(0);
	const int y = get_global_id(1);
	const int width = get_image_width(res);
	const int height = get_image_height(res);

	const float x0 = ((xMinMax.y - xMinMax.x) * x / width + xMinMax.x) / scale + dx;
	const float y0 = ((yMinMax.y - yMinMax.x) * (height - y) / height + yMinMax.x) / scale + dy;

	float xi = 0.0f;
	float yi = 0.0f;
	int iter = 0;
	while (xi * xi + yi * yi <= 4 && iter < maxIter)
	{
		float xTemp = xi * xi - yi * yi + x0;
		yi = 2 * xi * yi + y0;
		xi = xTemp;
		iter++;
	}

	const float4 col = (float4)(iter / 256 * 5 + 127,
		iter % 256,
		127.0f, 1.0f);

	const float4 convCol = (float4)(col.xyz / 255.0f, 1.0f);

	write_imagef(res, (int2)(x, y), convCol);
}