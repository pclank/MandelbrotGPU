const float2 xMinMax = (float2)(-2.0f, 0.47f);
const float2 yMinMax = (float2)(-1.12f, 1.12f);
const float scale = 1.0f;
const int maxIter = 1000;

float4 lerp(float4 a, float4 b, float t)
{
	return (1.0f - t) * a + t * b;
}

float3 lerp3(float3 a, float3 b, float t)
{
	return (1.0f - t) * a + t * b;
}

kernel void test(__global int* test_buf)
{
	int x = get_global_id(0);
	//int y = get_global_id(1);

	test_buf[x] = x;
}

// Colors
const float pos[] = { 0.0f, 0.16f, 0.42f, 0.6425f, 0.8575f };
const float3 col[] = { (float3)(0, 7, 100),
				(float3)(32, 107, 203),
				(float3)(237, 255, 255),
				(float3)(255, 170, 0),
				(float3)(0, 2, 0)};

const float3 cols[] = { (float3)(66, 30, 15),
	(float3)(25, 7, 26),
	(float3)(9, 1, 47),
	(float3)(4, 4, 73),
	(float3)(0, 7, 100),
	(float3)(12, 44, 138),
	(float3)(24, 82, 177),
	(float3)(57, 125, 209),
	(float3)(134, 181, 229),
	(float3)(211, 236, 248),
	(float3)(241, 233, 191),
	(float3)(248, 201, 95),
	(float3)(255, 170, 0),
	(float3)(204, 128, 0),
	(float3)(153, 87, 0),
	(float3)(106, 52, 3)
};

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

kernel void CalculateIterCounts(read_only image2d_t res, float dx, float dy, float scale, global int* iterCounts)
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
	while (xi * xi + yi * yi <= (1 << 16) && iter < maxIter)
	{
		float xTemp = xi * xi - yi * yi + x0;
		yi = 2 * xi * yi + y0;
		xi = xTemp;
		iter++;
	}

	iterCounts[x + y * get_image_width(res)] = iter;
}

kernel void CalculateIterPerPixel(read_only image2d_t res, global int* iterCounts, global int* iterPerPixel)
{
	const int x = get_global_id(0);
	const int y = get_global_id(1);
	const int width = get_image_width(res);
	const int height = get_image_height(res);

	const int i = iterCounts[x + y * get_image_width(res)];
	//NumIterationsPerPixel[i]++;
	atomic_inc(&iterPerPixel[i]);
}

kernel void CalculateTotal(global int* iterPerPixel, global int* total)
{
	int tot = 0;
	for (int i = 0; i < maxIter; i++)
		tot += iterPerPixel[i];

	total[0] = tot;
}

kernel void MandelSmooth(write_only image2d_t res, float dx, float dy, float scale)
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
	while (xi * xi + yi * yi <= (1 << 16) && iter < maxIter)
	{
		float xTemp = xi * xi - yi * yi + x0;
		yi = 2 * xi * yi + y0;
		xi = xTemp;
		iter++;
	}

	float flIter = iter;
	// Used to avoid floating point issues with points inside the set.
	if (iter < maxIter)
	{
		// sqrt of inner term removed using log simplification rules.
		float log_zn = log(xi * xi + yi * yi) / 2;
		float nu = log(log_zn / log(2.0f)) / log(2.0f);
		// Rearranging the potential function.
		// Dividing log_zn by log(2) instead of log(N = 1<<8)
		// because we want the entire palette to range from the
		// center to radius 2, NOT our bailout radius.
		flIter = iter + 1 - nu;
	}

	//const int i = iter % 16;
	const int i = (int)floor(flIter) % 16;
	//const float3 col = (iter < maxIter && iter > 0) ? cols[i] : (float3)(0.0f);
	const float3 col1 = (iter < maxIter && iter > 0) ? cols[i] : (float3)(0.0f);
	const float3 col2 = (iter < maxIter && iter > 0) ? cols[i + 1] : (float3)(0.0f);
	const float3 col = lerp3(col1, col2, flIter - floor(flIter));

	const float4 convCol = (float4)(col.xyz / 255.0f, 1.0f);

	write_imagef(res, (int2)(x, y), convCol);
}

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |
CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

kernel void GaussianFilter(read_only image2d_t input, write_only image2d_t res)
{
	//int k[9] = { -1, -1, -1, -1, 9, -1, -1, -1, -1 };
	int k[9] = { 1, 2, 1, 2, 4, 2, 1, 2, 1 };
	int x = get_global_id(0);
	int y = get_global_id(1);
	float4 pixel =
		k[0] * read_imagef(input, sampler,
			(int2)(x - 1, y - 1)) +
		k[1] * read_imagef(input, sampler,
			(int2)(x, y - 1)) +
		k[2] * read_imagef(input, sampler,
			(int2)(x + 1, y - 1)) +
		k[3] * read_imagef(input, sampler,
			(int2)(x - 1, y)) +
		k[4] * read_imagef(input, sampler,
			(int2)(x, y)) +
		k[5] * read_imagef(input, sampler,
			(int2)(x + 1, y)) +
		k[6] * read_imagef(input, sampler,
			(int2)(x - 1, y + 1)) +
		k[7] * read_imagef(input, sampler,
			(int2)(x, y + 1)) +
		k[8] * read_imagef(input, sampler,
			(int2)(x + 1, y + 1));

	pixel = (float4)(pixel.xyz / 16, 1.0f);
	write_imagef(res, (int2)(x, y), pixel);
}