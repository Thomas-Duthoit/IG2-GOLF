#version 330

in vec3 fragNormal;
in vec4 fragColor;

out vec4 finalColor;

uniform vec3 lightDir;
uniform vec4 lightColor;
uniform vec4 ambient;
uniform vec4 colDiffuse; 

// cam pos pour les reflets avec le specular
uniform vec3 viewPos; 

void main()
{
    // Lambert -> lumière du soleil
    float diff = max(dot(fragNormal, -lightDir), 0.0);
    vec3 diffuse = lightColor.rgb * diff;
    
    // Hemispheric -> éclairage du ciel
    float skyWeight = max(fragNormal.y, 0.0);
    vec3 skyLight = vec3(0.15, 0.2, 0.3) * skyWeight; // teinte bleutée pour les ombres

    // Specular -> reflets
    vec3 viewDir = normalize(viewPos - fragNormal); 
    vec3 reflectDir = reflect(lightDir, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 20.0);
    vec3 specular = lightColor.rgb * spec * 0.4;

    // COMBINAISON
    vec3 lighting = ambient.rgb + diffuse + skyLight + specular;
    
    finalColor = vec4(colDiffuse.rgb * lighting, colDiffuse.a);
}