#define WIDTH (640 * 10)
#define HEIGHT (480 * 10)

#define MIN_REAL -2.0
#define MAX_REAL 1.0
#define MIN_IMAGINARY -1.1f
#define MAX_IMAGINARY (MIN_IMAGINARY + (MAX_REAL - MIN_REAL) *\
(HEIGHT) / (WIDTH))
//#define MAX_IMAGINARY 1.2
#define MAX_ITERATIONS 200

#define IMAGINARY_POS(y)\
(float)(MAX_IMAGINARY -\
(y) * ((MAX_IMAGINARY - MIN_IMAGINARY) / (float)((HEIGHT) - 1)))

#define REAL_POS(x)\
(float)(MIN_REAL + (x) * ((MAX_REAL - MIN_REAL) / (float)((WIDTH) - 1)))

inline void AtomicAdd(volatile __global float *source, const float operand) {
    union {
        unsigned int intVal;
        float floatVal;
    } newVal;
    union {
        unsigned int intVal;
        float floatVal;
    } prevVal;
    do {
        prevVal.floatVal = *source;
        newVal.floatVal = prevVal.floatVal + operand;
    } while (atomic_cmpxchg((volatile __global unsigned int *)source, prevVal.intVal, 
                newVal.intVal) != prevVal.intVal);
}

typedef union
{
	float3 vec;
	float arr[3];
} float3_;

__kernel void simulate_gravity( __global float3 *host_pos, __global float3_ *host_speed, const int length)
{
	const int i = get_global_id(0);

	const float delta_time = 1.f;
   	 // const float grav_constant = 6.67428e-11;
    	const float grav_constant = 1;
    	const float mass_of_sun = 2;
    	const float mass_grav = grav_constant * mass_of_sun * mass_of_sun;
    	const float distance_to_nearest_star = 50;


	if(i>=length) {
		return;
	}
	
		for (int j = 0; j < length; ++j)
        {
            if (i == j)
                continue;

            float3 pos_a = host_pos[i];
            float3 pos_b = host_pos[j];

            float dist_x = (pos_a.s0 - pos_b.s0) * distance_to_nearest_star;
            float dist_y = (pos_a.s1 - pos_b.s1) * distance_to_nearest_star;
            float dist_z = (pos_a.s2 - pos_b.s2) * distance_to_nearest_star;


            float distance = sqrt(
                    dist_x * dist_x +
                    dist_y * dist_y +
                    dist_z * dist_z);

            float force_x = -mass_grav * dist_x / (distance * distance * distance);
            float force_y = -mass_grav * dist_y / (distance * distance * distance);
            float force_z = -mass_grav * dist_z / (distance * distance * distance);

            float acc_x = force_x / mass_of_sun;
            float acc_y = force_y / mass_of_sun;
            float acc_z = force_z / mass_of_sun;

           // host_speed[i].s0 += acc_x * delta_time;
           // host_speed[i].s1 += acc_y * delta_time;
           // host_speed[i].s2 += acc_z * delta_time;

			AtomicAdd(&host_speed[i].arr[0], (float)(acc_x * delta_time));
    		AtomicAdd(&host_speed[i].arr[1], (float)acc_y * delta_time);
    		AtomicAdd(&host_speed[i].arr[2], (float)acc_z * delta_time);



        }

      	  host_pos[i].s0 += (host_speed[i].vec.s0* delta_time) / distance_to_nearest_star;
      	  host_pos[i].s1 += (host_speed[i].vec.s1  * delta_time) / distance_to_nearest_star;
       	  host_pos[i].s2 += (host_speed[i].vec.s2  * delta_time) / distance_to_nearest_star;
   		
}

