#version 450

layout(location = 0) in vec3 texCoords;

layout(push_constant) uniform PushConstantBlock {
	mat4 NDCtoWorld;
	vec3 cameraPos;
} pushConstantBlock;

layout(binding = 0, rgba32f) uniform readonly image3D brick;

layout(location = 0) out vec4 outColor;

float piOver2 = asin(1.0);
vec4 skyColor = vec4(0.529, 0.808, 0.922, 1.0);
vec4 horizonColor = vec4(0.8,0.9,1.0, 1.0);
vec3 octree_location = vec3(-1.0,-1.0,-1.0);
float octree_scale = 2.0;
int brick_size = 32;

vec4 missColor(vec3 direction){
    float dotProd = dot(direction, vec3(0.0,0.0,1.0));
    dotProd = clamp(dotProd, -1.0, 1.0);
    float theta = acos(dotProd) / piOver2;

    if(theta < 1){ // sky (pi/2)
        return skyColor*(1-theta) + horizonColor*theta;
    }
    else{
	    return horizonColor*(2-theta);
    }
}

void main() {
     vec3 rayOrigin = pushConstantBlock.cameraPos;
	vec4 transformed = pushConstantBlock.NDCtoWorld * vec4(texCoords, 1.0);
    vec3 pixelLocation = transformed.xyz / transformed.w;
	vec3 direction = normalize(pixelLocation-rayOrigin);

    vec3 invDir = 1.0 / direction; // Inverse direction to handle division by zero
    vec3 tMin = (-1.0 - rayOrigin) * invDir;
    vec3 tMax = (1.0 - rayOrigin) * invDir;

    // Swap values if necessary so tMin always holds the entry points and tMax the exit points
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);

    // Find the largest tMin and the smallest tMax
    float tEntry = max(max(t1.x, t1.y), t1.z);
    float tExit = min(min(t2.x, t2.y), t2.z);

    bvec3 step_direction = bvec3(tEntry == t1.x, tEntry == t1.y, tEntry == t1.z);
    // If the largest entry point is greater than the smallest exit point, the ray misses the cube
    if (tEntry <= tExit && tExit >= 0.0){

        bvec3 advance = bvec3(direction.x >= 0, direction.y >= 0, direction.z >= 0);
        
        vec3 world_point = rayOrigin + direction * (tEntry+0.001);
        if(tEntry < 0.0) world_point = rayOrigin;


        vec3 octree_point = (world_point - octree_location) / octree_scale;
        vec3 voxel_point = octree_point*float(brick_size);
        ivec3 voxel_coord = ivec3(floor(voxel_point));

        int i = 0;
        while(i < 500){
            

            if(voxel_coord.x < 0 || voxel_coord.y < 0 || voxel_coord.z < 0 || voxel_coord.x >= brick_size || voxel_coord.y >= brick_size || voxel_coord.z >= brick_size){
				outColor = missColor(direction) - vec4(vec3(i / 250.0), 0.0);
                return;
			}

            vec4 voxel = imageLoad(brick, voxel_coord);
            if(voxel != vec4(0.0)){
                outColor = vec4(vec3(1.0)-vec3(step_direction)*0.1, 1.0)  - vec4(vec3(i / 250.0), 0.0);
				return;
			}
            
//            outColor = vec4(vec3(voxel_coord)/float(brick_size), 1.0);
//            return;

            tMin = (voxel_coord - voxel_point) * invDir;
            tMax = (voxel_coord + 1.0 - voxel_point) * invDir;
            t1 = min(tMin, tMax);
            t2 = max(tMin, tMax);
            //tEntry = max(max(t1.x, t1.y), t1.z);
            tExit = min(min(t2.x, t2.y), t2.z);
            step_direction = bvec3(tExit == t2.x, tExit == t2.y, tExit == t2.z);
            voxel_coord += ivec3(step_direction) * (ivec3(-1)+2*ivec3(advance));

            voxel_point = voxel_point + direction * tExit;
            i++;
        }      
    }
	else{
        outColor = missColor(direction);
	}
}