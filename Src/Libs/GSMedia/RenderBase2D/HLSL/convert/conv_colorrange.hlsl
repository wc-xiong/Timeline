#ifndef _CONV_COLORRANGE_
#define _CONV_COLORRANGE_

//fullRange Ey ----- (0-1) Ecb£¬Ecr ---- (-0.5 - +0.5)
//limitRange Y' Cb' Cr' (0-1)          
//Y' = ((219 * Ey + 16) * 2 ^ (n ? 8)) / (2 ^ n -1 )
//Cb' = ((224 * Ecb + 128) * 2 ^ (n ? 8)) / (2 ^ n -1 )
//Cr' = ((224 * Ecr + 128) * 2 ^ (n ? 8)) / (2 ^ n -1 )

#define CLR_RANGE_LIMIT 0
#define CLR_RANGE_FULL 1

//#if defined(value_range_8bit_mpeg)
//    yuv = (yuv - vec3(16.0 / 255.0)) * vec3(256.0 / 220.0, 256.0 / 225.0, 256.0 / 225.0);
//#elif defined(value_range_10bit_mpeg)
//    yuv = (yuv - vec3(64.0 / 1023.0)) * vec3(1024.0 / 877.0, 1024.0 / 897.0, 1024.0 / 897.0);
//#endif

float3 limit2full_8bit(const float3 yuv)
{
    float3 ret;

    //ret[0] = 1.164 * (yuv[0] - 0.0627451); 
    //ret[1] = 1.143 * (yuv[1] - 0.5019608);  
    //ret[2] = 1.143 * (yuv[2] - 0.5019608);
    
    //ret[0] = (255 * yuv[0] - 16) / 219;
    //ret[1] = (255 * yuv[1] - 128) / 224;
    //ret[2] = (255 * yuv[2] - 128) / 224;
    
    ret = (yuv - float3(16.0 / 255.0, 16.0 / 255.0, 16.0 / 255.0)) * float3(255.0 / 219.0, 255.0 / 224.0, 255.0 / 224.0);

    return ret;
}

float3 limit2full_10bit(const float3 yuv)
{
    float3 ret;

    //ret[0] = 1.168 * (yuv[0] - 0.0625611); 
    //ret[1] = 1.143 * (yuv[1] - 0.5004888);
    //ret[2] = 1.143 * (yuv[2] - 0.5004888); 
    
    //ret[0] = (1023 * yuv[0] / 4 - 16) / 219;
    //ret[1] = (1023 * yuv[1] / 4 - 128) / 224;
    //ret[2] = (1023 * yuv[2] / 4 - 128) / 224;
    
    ret = (yuv - float3(64.0 / 1023.0, 64.0 / 1023.0, 64.0 / 1023.0)) * float3(1023.0 / 876.0, 1023.0 / 896.0, 1023.0 / 896.0);

    return ret;
}

#endif