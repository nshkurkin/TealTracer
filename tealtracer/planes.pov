// plane test (RIGHT HANDED)

camera {
  location  <0, 0, 16>
  up        <0,  1,  0>
  right     <1.33333, 0,  0>
  look_at   <0, 0, 0>
}

light_source {<0, 4, 0> color rgb <1.0, 1.0, 1.0>}

//right wall - note distance is along normal thus to move right is negative for this normal
plane {<-1, 0, 0>, -6 
      pigment {color rgb <0.5, 0.8, 0.2>}
      finish {ambient 0.0 diffuse 1.0}
}

//bottom
plane {<0, 1, 0>, -6
      pigment {color rgb <0.15 0.2, 0.8>}
      finish {ambient 0.0 diffuse 1.0}
}

//top
plane {<0, -1, 0>, -8
      pigment {color rgb <0.15 0.2, 0.8>}
      finish {ambient 0.0 diffuse 1.0}
}

//back
plane {<0, 0, 1>, -6
      pigment {color rgb <0.5, 0.5, 0.5>}
      finish {ambient 0.0 diffuse 1.0}
}

//front
plane {<0, 0, -1>, -17
      pigment {color rgb <0.2, 0.2, 0.8>}
      finish {ambient 0.0 diffuse 1.0}
}

//left
plane {<1, 0, 0>, -5
      pigment {color rgb <0.80 0.2, 0.2>}
      finish {ambient 0.0 diffuse 1.0}
}

//left sphere
sphere { <-5, 1, 0>, 1.1
  pigment { color rgb <1.0, 1.0, 1.0>}
  finish {ambient 0.0 diffuse 1.0}
}

//left sphere
sphere { <3, -4.5, 0>, 0.7
  pigment { color rgb <1.0, 1.0, 1.0>}
  finish {ambient 0.0 diffuse 1.0}
}
