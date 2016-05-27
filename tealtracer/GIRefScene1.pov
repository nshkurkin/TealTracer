//Scene with fully reflective and refractive spheres

camera {
   location  <0.0, 0.0, 1.0>
   up        <0,  1,  0>
   right     <1.33333, 0,  0>
   look_at   <0.0, 0.0, 0.0>
}

light_source {<0.0, 2.7, -7.0> color rgb <1.0, 1.0, 1.0>}

sphere { <1.0, 0, -7.0>, 1.0
   pigment { color rgb <1.0, 1.0, 1.0>}
   finish {ambient 0.2 diffuse 1.0 ior 1.5 refraction 1.0 preflect 0.0 prefract 1.0}
}

sphere { <-1.5, -2.0, -10.0>, 1.0
   pigment { color rgb <1.0, 1.0, 1.0>}
   finish {ambient 0.2 diffuse 1.0 ior 1.5 reflection 1.0 preflect 1.0 prefract 0.0}
}

// Floor
plane {<0.0, 1.0, 0.0>, -3.0 
   pigment {color rgb <0.7, 0.7, 0.7>}
   finish {ambient 0.2 diffuse 1.0 ior 1.5 preflect 0.5}
}

// Ceiling
plane {<0.0, -1.0, 0.0>, -3.5 
   pigment {color rgb <0.7, 0.7, 0.7>}
   finish {ambient 0.2 diffuse 1.0 ior 1.5 preflect 0.5}
}

// Left Wall
plane {<1.0, 0.0, 0.0>, -3.5 
   pigment {color rgb <1.0, 0.2, 0.2>}
   finish {ambient 0.2 diffuse 1.0 ior 1.5 preflect 0.5}
}

// Right Wall
plane {<-1.0, 0.0, 0.0>, -3.5 
   pigment {color rgb <0.2, 1.0, 0.2>}
   finish {ambient 0.2 diffuse 1.0 ior 1.5 preflect 0.5}
}

// Back Wall
plane {<0.0, 0.0, 1.0>, -13.0 
   pigment {color rgb <0.7, 0.7, 0.7>}
   finish {ambient 0.2 diffuse 1.0 ior 1.5 preflect 0.5}
}

// Front Wall
plane {<0.0, 0.0, -1.0>, -1.5 
   pigment {color rgb <0.7, 0.7, 0.7>}
   finish {ambient 0.2 diffuse 1.0 ior 1.5}
}
