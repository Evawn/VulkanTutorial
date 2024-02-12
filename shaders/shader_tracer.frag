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

    // If the largest entry point is greater than the smallest exit point, the ray misses the cube
    if (tEntry <= tExit && tExit >= 0.0){

        vec3 advance = direction / abs(direction);
        vec3 world_point = rayOrigin + direction * tEntry;
        vec3 octree_point = (world_point - octree_location) / octree_scale;

        vec3 voxel_point = octree_point*float(brick_size);

        vec4 temp = imageLoad(brick, ivec3(0,0,0));
        if(temp != vec4(0.0)){
			outColor = vec4(1.0,1.0,1.0,1.0);
			return;
		}
        else{
            outColor = vec4(0.0,0.0,0.0,1.0);
        }

        int i = 0;
        while(i < 500){
            vec3 voxel_coord = floor(voxel_point);

            if(voxel_coord.x < 0 || voxel_coord.y < 0 || voxel_coord.z < 0 || voxel_coord.x >= brick_size || voxel_coord.y >= brick_size || voxel_coord.z >= brick_size){
				outColor = missColor(direction);
                return;
			}

            vec4 voxel = imageLoad(brick, ivec3(voxel_coord));
            if(voxel != vec4(0.0)){
                outColor = vec4(1.0,1.0,1.0,1.0);
				return;
			}

            tMin = (voxel_coord - voxel_point) * invDir;
            tMax = (voxel_coord + 1.0 - voxel_point) * invDir;
            t1 = min(tMin, tMax);
            t2 = max(tMin, tMax);
            tEntry = max(max(t1.x, t1.y), t1.z);
            tExit = min(min(t2.x, t2.y), t2.z);

            voxel_point = voxel_point + direction * tExit;
            i++;
        }
        outColor = vec4(1.0,0.0,0.0,1.0);
       
        //outColor = vec4(voxel_coord/float(brick_size),1.0);
        
    }
	else{
        outColor = missColor(direction);
	}
}