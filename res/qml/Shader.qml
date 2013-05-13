import QtQuick 2.0
import Qt3D 2.0

ShaderProgram {

    id: program
    texture: "qrc:/textures/layout"

    property variant offAxisProjectionMatrix: []

    vertexShader: "
        #version 130

        in highp vec4 qt_Vertex;
        in highp vec4 qt_MultiTexCoord0;
        in highp vec3 qt_Normal;

        uniform mediump mat4 offAxisProjectionMatrix;
        uniform mediump mat4 qt_ModelViewMatrix;
        uniform highp mat3 qt_NormalMatrix;

        struct qt_MaterialParameters {
            mediump vec4 emission;
            mediump vec4 ambient;
            mediump vec4 diffuse;
            mediump vec4 specular;
            mediump float shininess;
        };
        uniform qt_MaterialParameters qt_Material;

        struct qt_SingleLightParameters {
            mediump vec4 position;
            mediump vec3 spotDirection;
            mediump float spotExponent;
            mediump float spotCutoff;
            mediump float spotCosCutoff;
            mediump float constantAttenuation;
            mediump float linearAttenuation;
            mediump float quadraticAttenuation;
        };
        uniform qt_SingleLightParameters qt_Light;

        out highp vec4 texCoord;
        out mediump vec4 litColor;
        out mediump vec4 litSecondaryColor;

        void main(void)
        {
            texCoord = qt_MultiTexCoord0;

            // Calculate the vertex and normal to use for lighting calculations.
            highp vec4 vertex = qt_ModelViewMatrix * qt_Vertex;
            highp vec3 normal = normalize(qt_NormalMatrix * qt_Normal);

            gl_Position = offAxisProjectionMatrix * vertex;

            // Start with the material's emissive color and the ambient scene color,
            // which have been combined into the emission parameter.
            vec4 color = qt_Material.emission;

            vec4 scolor = qt_Material.specular;

            // Viewer is at infinity.
            vec3 toEye = vec3(0, 0, 1);

            // Determine the angle between the normal and the light direction.
            vec4 pli = qt_Light.position;
            vec3 toLight;
            if (pli.w == 0.0)
                toLight = normalize(pli.xyz);
            else
                toLight = normalize(pli.xyz - vertex.xyz);
            float angle = max(dot(normal, toLight), 0.0);

            // Calculate the ambient and diffuse light components.
            vec4 adcomponent = qt_Material.ambient + angle * qt_Material.diffuse;

            // Calculate the specular light components.
            vec4 scomponent;
            if (angle != 0.0) {
                vec3 h = normalize(toLight + toEye);
                angle = max(dot(normal, h), 0.0);
                float srm = qt_Material.shininess;
                vec4 scm = qt_Material.specular;
                if (srm != 0.0)
                    scomponent = pow(angle, srm) * scm;
                else
                    scomponent = scm;
            } else {
                scomponent = vec4(0, 0, 0, 0);
            }

            // Apply the spotlight angle and exponent.
            if (qt_Light.spotCutoff != 180.0) {
                float spot = max(dot(normalize(vertex.xyz - pli.xyz),
                             qt_Light.spotDirection), 0.0);
                if (spot < qt_Light.spotCosCutoff) {
                    adcomponent = vec4(0, 0, 0, 0);
                    scomponent = vec4(0, 0, 0, 0);
                } else {
                    spot = pow(spot, qt_Light.spotExponent);
                    adcomponent *= spot;
                    scomponent *= spot;
                }
            }

            // Apply attenuation to the colors.
            if (pli.w != 0.0) {
                float attenuation = qt_Light.constantAttenuation;
                float k1 = qt_Light.linearAttenuation;
                float k2 = qt_Light.quadraticAttenuation;
                if (k1 != 0.0 || k2 != 0.0) {
                    float len = length(pli.xyz - vertex.xyz);
                    attenuation += k1 * len + k2 * len * len;
                }
                color += adcomponent / attenuation;
                scolor += scomponent / attenuation;
            } else {
                color += adcomponent;
                scolor += scomponent;
            }

            // Generate the final output colors to pass to the fragment shader.
            float alpha = qt_Material.diffuse.a;
            litColor = vec4(clamp(color.rgb, 0.0, 1.0), alpha);
            litSecondaryColor = vec4(clamp(scolor.rgb, 0.0, 1.0), 0.0);
        }
       "
    fragmentShader: "
        #version 130

        uniform sampler2D qt_Texture0;
        uniform sampler2D image;

        in highp vec4 texCoord;
        in mediump vec4 litColor;
        in mediump vec4 litSecondaryColor;

        void main(void)
        {
            mediump vec4 imageColor = litColor * texture2D(image, texCoord.st);
            gl_FragColor = clamp(imageColor + litSecondaryColor, 0.0, 1.0);
        }
        "
}
