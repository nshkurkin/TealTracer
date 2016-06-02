// cs174, assignment 1 sample file (RIGHT HANDED)

camera {
  location  <0, 0, 14>
  up        <0,  1,  0>
  right     <1.33333, 0,  0>
  look_at   <0, 0, 0>
}


light_source {<-10, 10, 10> color rgb <1.5, 1.5, 1.5>}

sphere { <0, 0, 0>, 2
  pigment { color rgb <1.0, 0.0, 1.0>}
  finish {ambient 0.0 diffuse 1.0}
  translate <0, 0, 0>
}

plane {<0, 1, 0>, -4
      pigment {color rgb <0.2, 0.2, 0.8>}
      finish {ambient 0.0 diffuse 1.0}
}