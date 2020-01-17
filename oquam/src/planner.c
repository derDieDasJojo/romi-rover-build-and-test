/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include <r.h>
#include <float.h>
#include "v.h"
#include "planner.h"
#include "plotter.h"


/**************************************************************************/

static inline double max(double a, double b)
{
        return (a > b)? a : b;
}

static inline double min(double a, double b)
{
        return (a < b)? a : b;
}

static inline double sign(double a)
{
        return (a < 0.0)? -1.0 : 1.0;
}

/**************************************************************************/

static void delete_section_list(list_t *list)
{
        for (list_t *l = list; l; l = list_next(l)) {
                section_t *section = list_get(l, section_t);
                delete_section(section);
        }
        delete_list(list);
}

section_t *new_section()
{
        section_t *section = r_new(section_t);
        if (section == NULL)
                return NULL;
        return section;
}

static section_t *new_section_init(double t, double at,
                                   double *p0, double *p1,
                                   double *v0, double *v1,
                                   double *a)
{
        section_t *section = new_section();
        if (section == NULL)
                return NULL;

        section->t = t;
        section->at = at;
        vcopy(section->p0, p0);
        vcopy(section->p1, p1);
        vcopy(section->v0, v0);
        vcopy(section->v1, v1);
        vcopy(section->a, a);
        vsub(section->d, section->p1, section->p0);
        
        return section;
}

void delete_section(section_t *section)
{
        if (section) {
                for (list_t *l = section->actions; l; l = list_next(l)) {
                        action_t *a = list_get(l, action_t);
                        delete_action(a);
                }
                delete_list(section->actions);
                r_delete(section);
        }
}

static list_t *section_slice(section_t *section, double period, double maxlen)
{
        list_t *slices = NULL;
        double tmp[3];        
        double T = period;
        
        /* The segment has a constant speed. Sample at distances
         * 'maxlen' instead of 'period'. */
        if (norm(section->a) == 0)
                T = maxlen;

        double t = 0.0;
                
        while (t < section->t) {
                double dt = section->t - t;
                if (dt > T)
                        dt = T;
                
                double v0[3];
                smul(v0, section->a, t);
                vadd(v0, section->v0, v0);
                
                double p0[3];
                smul(tmp, section->v0, t);
                smul(p0, section->a, 0.5 * t * t);
                vadd(p0, p0, tmp);
                vadd(p0, p0, section->p0);
                
                double v1[3];
                smul(v1, section->a, t+dt);
                vadd(v1, section->v0, v1);
                
                double p1[3];
                smul(tmp, section->v0, t+dt);
                smul(p1, section->a, 0.5 * (t+dt) * (t+dt));
                vadd(p1, p1, tmp);
                vadd(p1, p1, section->p0);
                
                section_t *s = new_section_init(dt, section->at + t,
                                                p0, p1, v0, v1,
                                                section->a);
                if (s == NULL) {
                        delete_section_list(slices);
                        return NULL;
                }

                slices = list_append(slices, s);
                if (slices == NULL)
                        return NULL;                
                
                t += dt;
        }
        
        return slices;
}

static void section_print(section_t *section, const char *prefix)
{
        printf("%s: ", prefix);
        printf("p0(%0.3f,%0.3f,%0.3f)-p1(%0.3f,%0.3f,%0.3f); ",
               section->p0[0], section->p0[1], section->p0[2],
               section->p1[0], section->p1[1], section->p1[2]);
        printf("v0(%0.3f,%0.3f,%0.3f)-v1(%0.3f,%0.3f,%0.3f); ",
               section->v0[0], section->v0[1], section->v0[2],
               section->v1[0], section->v1[1], section->v1[2]);
        printf("a(%0.3f,%0.3f,%0.3f)",
               section->a[0], section->a[1], section->a[2]);
        printf("\n");
}

/**************************************************************************/

void delete_segment(segment_t *segment);

segment_t *new_segment()
{
        segment_t *segment = r_new(segment_t);
        if (segment == NULL)
                return NULL;
        return segment;
}

void delete_segment(segment_t *segment)
{
        if (segment) {
                for (list_t *l = segment->actions; l; l = list_next(l)) {
                        action_t *a = list_get(l, action_t);
                        delete_action(a);
                }
                delete_list(segment->actions);
                r_delete(segment);
        }
}

static void segments_delete(segment_t *path)
{
        segment_t *s = path;
        while (s != NULL) {
                segment_t *next = s->next;
                delete_segment(s);
                s = next;
        }
}

static void segment_list_delete(list_t *paths)
{
        for (list_t *l = paths; l != NULL; l = list_next(l)) {
                segment_t *path = list_get(l, segment_t);
                segments_delete(path);
        }
        delete_list(paths);
}

/**************************************************************************/

void delete_atdc(atdc_t *atdc);

atdc_t *new_atdc()
{
        atdc_t *atdc = r_new(atdc_t);
        if (atdc == NULL)
                return NULL;
        return atdc;
}

void delete_atdc(atdc_t *atdc)
{
        if (atdc) {
                for (list_t *l = atdc->actions; l; l = list_next(l)) {
                        action_t *a = list_get(l, action_t);
                        delete_action(a);
                }
                delete_list(atdc->actions);
                r_delete(atdc);
        }
}

static list_t *atdc_slice(atdc_t *atdc, double period, double maxlen)
{
        list_t *list = NULL;
        
        if (atdc->accelerate.t > 0) {
                list_t *slices = section_slice(&atdc->accelerate, period, maxlen);
                if (slices == NULL) {
                        delete_section_list(list);
                        return NULL;
                }
                list = list_concat(list, slices);
        }
        
        if (atdc->travel.t > 0) {
                list_t *slices = section_slice(&atdc->travel, period, maxlen);
                if (slices == NULL) {
                        delete_section_list(list);
                        return NULL;
                }
                list = list_concat(list, slices);
        }
        
        if (atdc->decelerate.t > 0) {
                list_t *slices = section_slice(&atdc->decelerate, period, maxlen);
                if (slices == NULL) {
                        delete_section_list(list);
                        return NULL;
                }
                list = list_concat(list, slices);
        }
        
        if (atdc->curve.t > 0) {
                list_t *slices = section_slice(&atdc->curve, period, maxlen);
                if (slices == NULL) {
                        delete_section_list(list);
                        return NULL;
                }
                list = list_concat(list, slices);
        }

        if (atdc->actions != NULL) {
                // Add an empty section with only actions
                section_t *section = new_section();
                if (section == NULL)
                        return NULL;
                                
                // Clone the list of actions
                for (list_t *l = atdc->actions; l; l = list_next(l)) {
                        action_t *a = list_get(l, action_t);
                        section->actions = list_append(section->actions,
                                                       action_clone(a));
                }

                list = list_append(list, section);
        }
        
        return list;
}

static list_t *atdc_list_slice(atdc_t *atdc_list, double period, double maxlen)
{
        list_t *list = NULL;
        
        for (atdc_t *atdc = atdc_list; atdc != NULL; atdc = atdc->next) {
                list_t *slices = atdc_slice(atdc, period, maxlen);
                if (slices == NULL) {
                        delete_section_list(list);
                        return NULL;
                }
                list = list_concat(list, slices);
        }
        
        return list;
}

static void atdc_print(atdc_t *atdc)
{
        atdc_t *s = atdc;
        while (s != NULL) {
                section_print(&s->accelerate, "A");
                section_print(&s->travel, "T");
                section_print(&s->decelerate, "D");
                section_print(&s->curve, "C");
                s = s->next;
                if (s)
                        printf("-\n");
        }
}

static void atdc_delete(atdc_t *atdc)
{
        atdc_t *s = atdc;
        while (s != NULL) {
                atdc_t *next = s->next;
                delete_atdc(s);
                s = next;
        }
}

static void atdc_list_delete(list_t *atdc_list)
{
        for (list_t *l = atdc_list; l != NULL; l = list_next(l)) {
                atdc_t *atdc = list_get(l, atdc_t);
                atdc_delete(atdc);
        }
        delete_list(atdc_list);
}

/**************************************************************************/

struct _planner_t {
        cnc_t *cnc;
        controller_t *controller;

        list_t *segments;
        list_t *atdc_list;
};

static int planner_convert_script(planner_t *planner, script_t *script);

planner_t *new_planner(cnc_t *cnc, controller_t *controller, script_t *script)
{
        planner_t *planner = r_new(planner_t);
        if (planner == NULL)
                return NULL;

        planner->cnc = cnc;
        planner->controller = controller;

        if (planner_convert_script(planner, script) != 0) {
                delete_planner(planner);
                return NULL;
        }
        
        return planner;
}

void delete_planner(planner_t *planner)
{
        if (planner) {
                segment_list_delete(planner->segments);
                atdc_list_delete(planner->atdc_list);
                r_delete(planner);
        }
}

static void segment_compute_curve(segment_t *s0, atdc_t *t0, double d, double *amax)
{
        segment_t *s1 = s0->next;
        atdc_t *t1 = t0->next;

        if (s1 == NULL) {
                t0->curve.t = 0.0;
                vzero(t0->curve.v0);
                vzero(t0->curve.v1);
                vcopy(t0->curve.p0, s0->section.p1);
                vcopy(t0->curve.p1, s0->section.p1);
                vzero(t0->curve.a);
                vzero(t0->curve.d);
                return;
        }

        // Set the entry and exit speed of the curve to smallest speed
        // before and after the junction.
        double w = min(norm(s0->section.v0), norm(s1->section.v0));

        // The speed - and direction - before and after the curve.
        double w0[3];
        double w1[3];
        smul(w0, normalize(w0, s0->section.d), w);
        smul(w1, normalize(w1, s1->section.d), w);
        
        /*

          We have to compute the maximum speed at the entry of the
          curve such that the deviation at the junction is less than
          the maximum allowable deviation d. The maximim acceleration
          that can be applied is 'amax'.

          We also have to compute the points at which the controller
          has to start and stop accelerating. We'll call these points
          q0(x0,y0) and q1(x1, y1).

          In the discussion below, we change the coordinate system. We
          place the curve in the plane that goes through _w0 and _w1
          (I'm using underscore to denote vectors). The y-axis is
          positioned half-way between the _w0 and _w1 vectors.

          We have:
          _wx = 1/2 (_w0 + _w1), _ex = _vx / |_vx| 
          _wy = 1/2 (_w0 - _w1), _ey = _vy / |_vy|

                  y
                  |          
           w0 \   |   / w1       
               \  |  /        
                \ | /         
                 \|/         
           -------o--->--- x
                  |   wx
                  |   
                  |   
                  v wy

           The origin is placed in the junction point. In this frame
           of reference, we have for _w0(wx0, wy0, wz0) and _w1(wx1,
           wy1, wz1) that wx0=wx1, wy0=-wy1, and wz0=wz1=0.

        */
        
        // The x and y unit vectors in the new coordinate space (ex,
        // ey). The wx0 and wy0 are the speeds in these directions.
        double ex[3] = { 0.0, 0.0, 0.0 };
        vadd(ex, w1, w0);
        smul(ex, ex, 0.5);
        double wx0 = norm(ex);
        smul(ex, ex, 1/wx0);

        double ey[3] = { 0.0, 0.0, 0.0 };;
        vsub(ey, w0, w1);
        smul(ey, ey, 0.5);        
        double wy0 = norm(ey);
        smul(ey, ey, -1/wy0);


        /* Compute the maximum allowed acceleration in the direction
           of wy (or ey). */
        double a[3];
        
        if (1) {

                double s = DBL_MAX;
                for (int i = 0; i < 3; i++) {
                        if (ey[i] != 0.0)
                                s = min(s, amax[i] / fabs(ey[i]));
                }
                vcopy(a, ey);
                smul(a, a, s);
                        
                
        } else {
        
                /*
                  The Ellipsoid describing the maximum accelerations is:
                  ax²/amax_x² + ay²/amax_y² + az²/amax_z² = 1

                  We need to know where the line (0,ey) crosses the ellipsoid.
                  The points on the line satisfy the following:
                  p(x,y,z) = (x, (ey_y/ey_x).x, (ey_z/ey_x).x)
          
                  Substituting the parametric values for y and z in the
                  equation for the ellipsoid and solving for x gives the value
                  for x below.
          
                */
        
                // First, handle some simple cases
                if (ey[0] == 0.0 && ey[1] == 0.0) {
                        a[2] = amax[2];
                } else if (ey[0] == 0.0 && ey[2] == 0.0) {
                        a[1] = amax[1];
                } else if (ey[1] == 0.0 && ey[2] == 0.0) {
                        a[0] = amax[0];
                } else if (ey[2] == 0.0) {
                        // An ellepsis and line in the xy plane
                        a[0] = ((ey[0] * amax[0] * amax[1])
                                / sqrt((ey[0] * ey[0] * amax[1] * amax[1])
                                       + (ey[1] * ey[1] * amax[0] * amax[0])));
                        a[1] = ey[1] * a[0] / ey[0];

                } else if (ey[1] == 0.0) {
                        // An ellepsis and line in the xz plane
                        a[0] = ((ey[0] * amax[0] * amax[2])
                                / sqrt((ey[0] * ey[0] * amax[2] * amax[2])
                                       + (ey[2] * ey[2] * amax[0] * amax[0])));
                        a[2] = ey[2] * a[1] / ey[1];

                } else if (ey[0] == 0.0) {
                        // An ellepsis and line in the yz plane
                        a[1] = ((ey[1] * amax[1] * amax[2])
                                / sqrt((ey[1] * ey[1] * amax[2] * amax[2])
                                       + (ey[2] * ey[2] * amax[1] * amax[1])));
                        a[2] = ey[2] * a[1] / ey[1];
                
                } else {        
                        a[0] = ((ey[0] * amax[0] * amax[1] * amax[2])
                                / sqrt((ey[0] * ey[0] * amax[1] * amax[1] * amax[2] * amax[2])
                                       + (ey[1] * ey[1] * amax[0] * amax[0] * amax[2] * amax[2])
                                       + (ey[2] * ey[2] * amax[0] * amax[0] * amax[1] * amax[1])));
                        a[1] = ey[1] * a[0] / ey[0];
                        a[2] = ey[2] * a[0] / ey[0];
                }
        }

        
        // This is the maximum acceleration that we can tolerate along
        // the ey axis.
        double am = norm(a);

        /*
          The acceleration is applied only along the y-axis, a =
          ay. The absolute speed at the entry of the curve is the same
          as the absolute speed at the exit, |_w0|=|_w1|. The speed
          along the x-axis remain the same, wx0 = vx1, and the speed
          along the y-axis inverses, wy1 = -wy0. wy0 and ay have
          opposite signs.

          We have:
          wx = wx0 = wx1 = |_wx| 
          wy = wy0 = -wy1 = |_wy|

          ∆wy = wy1 - wy0 = -|_w0 - _w1| 
           
          The equation for the speed is (wy0 and a have opposite
          signs):
          wy1 = wy0 + a·∆t => ∆t = -2wy0/a                      (1)
          
          The equation for the y position is:
          y = y0 + wy0·t + a.t²/2

          When t = ∆t/2, y reaches its minimim ym  
          => ym = y0 + wy0·∆t/2 + a.(∆t/2)²/2, using (1) and develop
          => ym = y0 - wy0²/2a                                    (2)
          
          The time it takes to follow the speed curve is the same as
          the time it takes to follow the two segments of the original
          path that go through the junction point. This follows from
          the fact the the speed along the x-axis remains constant in
          both cases. Following the orginal straight path for ∆t/2,
          the junction at 0 is reached:
          
          y0 + wy0·∆t/2 = 0 => y0 = wy0²/a                        (3)
          
          (2) and (3) combined gives: ym = wy0²/2a
          
          The error ym should be smaller than the maximim deviation d.
          ym < d => wy0²/2a < d => wy0 < sqrt(2ad)               
          
          If the requested speed is larger than the maximum, all
          speed components will be scaled linearly.
          
          We already calculated the y coordinate of the entry and exit
          points in (3). The x coordinates of the entry and exit
          points are:
          
          ∆x = wx0·∆t => ∆x = -2wy0.wx0/a,            using (1)           
          => x0 = -∆x/2 = wy0.wx0/a and x1 = ∆x/2 = -wy0·wx0/a

          To obtain the acceleration to apply on the stepper motors,
          we have to rotate the acceleration (0,ay,0) back into the
          coordinate space of the CNC. 

         */
        
        // Check for maximum velocity wymax at the entry point, given
        // the maximum acceleration and deviation. If the current
        // speed is higher than wymax then scale the entry velocity
        // proportionally.
        double wymax = sqrt(2.0 * am * d);
        double vscale = 1.0;
        
        if (fabs(wy0) > wymax)
                vscale = wymax / wy0;

        wx0 *= vscale;
        wy0 *= vscale;
        smul(w0, w0, vscale);
        smul(w1, w1, vscale);
        
        // Compute the entry and exit coordinates 
        double x0 = wy0 * wx0 / am;
        double y0 = wy0 * wy0 / am;

        // The distance between the entry/exit points and the
        // junction.
        double lq = sqrt(x0 * x0 + y0 * y0);

        // If the entry or exit point is more than halfway along the
        // segment then we should slow down.
        double len0 = norm(s0->section.d);
        double len1 = norm(s1->section.d);
        double len = min(len0, len1);
        
        if (lq > len / 2.0) {
                vscale = len / (2.0 * lq);
                double sqvs = sqrt(vscale);
                x0 *= vscale;
                y0 *= vscale;
                wx0 *= sqvs;
                wy0 *= sqvs;
                smul(w0, w0, sqvs);
                smul(w1, w1, sqvs);
                lq = len / 2.0;
        }

        vcopy(t0->curve.v0, w0);                                 // v0
        vcopy(t0->curve.v1, w1);                                 // v1

        // FYI, The deviation is:
        //s0->deviation = 0.5 * wy0 * wy0 / am; 
        
        // Compute a vector along d0 with length lq
        double p0[3];
        normalize(p0, s0->section.d);
        smul(p0, p0, lq);

        /* The absolute position of the entry point of the speed
         * curve. */
        vsub(p0, s0->section.p1, p0);
        vcopy(t0->curve.p0, p0);                                 // p0

        
        // Compute a vector along d1 with length lq
        double p1[3];
        normalize(p1, s1->section.d);
        smul(p1, p1, lq);

        /* The absolute position of the exit point of the speed
         * curve */
        vadd(p1, s0->section.p1, p1);
        vcopy(t0->curve.p1, p1);                                 // p1

        /* Copy the next segment's entry speed and entry point to its
         * accelerate section */
        vcopy(t1->accelerate.v0, t0->curve.v1);
        vcopy(t1->accelerate.p0, p1);
        
        /* The acceleration, in the coordinate space of the CNC
         * machine. */
        vcopy(t0->curve.a, a);                                   // a
        
        /* The time it takes to follow the speed curve. */
        t0->curve.t = fabs(2.0 * wy0 / am);                      // t
}

static void segment_compute_accelerations(segment_t *s0, atdc_t *t0,
                                          double *amax, double at)
{
        double dv[3];
        double dt[3];
        double dx[3];
        double a[3];
        double tmp[3];
        
        double v = norm(s0->section.v0);

        vsub(dx, t0->curve.p0, t0->accelerate.p0);
        double len = norm(dx);

        /* Accelerate */
        
        // p0 and v0 are already set
        vsub(dv, s0->section.v0, t0->accelerate.v0);
        vdiv(dt, dv, amax);
        vabs(dt, dt);
        t0->accelerate.t = vmax(dt);                             // t
        if (t0->accelerate.t == 0) {
                vzero(t0->accelerate.a);                         // a
                vcopy(t0->accelerate.v1, t0->accelerate.v0);     // v1
                vzero(t0->accelerate.d);                         // d
                vcopy(t0->accelerate.p1, t0->accelerate.p0);     // p1
        } else {
                sdiv(t0->accelerate.a, dv, t0->accelerate.t);    // a
                vcopy(t0->accelerate.v1, s0->section.v0);        // v1
                smul(dx, t0->accelerate.v0, t0->accelerate.t);
                smul(tmp, t0->accelerate.a, 0.5 * t0->accelerate.t * t0->accelerate.t);
                vadd(dx, dx, tmp);
                vcopy(t0->accelerate.d, dx);                     // d
                vadd(t0->accelerate.p1, t0->accelerate.p0, dx);  // p1
        }
        vsub(dx, t0->accelerate.p1, t0->accelerate.p0);
        double len_a = norm(dx);

        /* Decelerate */
        
        vcopy(t0->decelerate.v0, s0->section.v0);                // v0
        vcopy(t0->decelerate.p1, t0->curve.p0);                  // p1
        vcopy(t0->decelerate.v1, t0->curve.v0);                  // v1
        vsub(dv, t0->decelerate.v0, t0->decelerate.v1);
        vdiv(dt, dv, amax);
        vabs(dt, dt);
        t0->decelerate.t = vmax(dt);                             // t
        
        if (t0->decelerate.t == 0.0) {
                vzero(t0->decelerate.a);                         // a
                vzero(t0->decelerate.d);                         // d
                vcopy(t0->decelerate.p0, t0->decelerate.p1);     // p0
        } else {
                sdiv(t0->decelerate.a, dv, -t0->decelerate.t);    // a
                smul(dx, t0->decelerate.v0, t0->decelerate.t);
                smul(tmp, t0->decelerate.a, 0.5 * t0->decelerate.t * t0->decelerate.t);
                vadd(dx, dx, tmp);
                vcopy(t0->decelerate.d, dx);                     // d
                vsub(t0->decelerate.p0, t0->decelerate.p1, dx);  // p0
        }
        vsub(dx, t0->decelerate.p1, t0->decelerate.p0);
        double len_d = norm(dx);

        /* Travel at constant speed for the remaining lenght */
        if (len_a + len_d < len) {
                double len_t = len - len_a - len_d;
                t0->travel.t = len_t / v;                          // t
                vcopy(t0->travel.p0, t0->accelerate.p1);           // p0
                vcopy(t0->travel.p1, t0->decelerate.p0);           // p1
                vcopy(t0->travel.v0, s0->section.v0);              // v0
                vcopy(t0->travel.v1, s0->section.v0);              // v1
                vzero(t0->travel.a);                               // a
                vsub(t0->travel.d, t0->travel.p1, t0->travel.p0);  // d
                
        } else {
                // Update the accelerate and decelerate sections
                double v1 = norm(t0->curve.v0);

                /* The time it takes to slow down from v0 to v1 */
                vsub(dv, t0->accelerate.v0, t0->curve.v0);
                vdiv(dt, dv, amax);
                double dt1 = vmax(dt);
                sdiv(a, dv, dt1);
                
                /* The distance travelled during that time */
                smul(dx, t0->accelerate.v0, dt1);
                smul(tmp, a, -0.5 * dt1 * dt1);
                vadd(dx, dx, tmp);
                len_d = norm(dx);

                /* Set the acceleration to zero */
                
                /* NOTE: It is possible to accelerate for a short
                 * distance and decelerate early. However, it is
                 * simpler, and perhaps better for the motors, just to
                 * move at constant speed. */
                t0->accelerate.t = 0.0;                       // t
                vcopy(t0->accelerate.p1, t0->accelerate.p0);  // p1
                vcopy(t0->accelerate.v1, t0->accelerate.v0);  // v1
                vzero(t0->accelerate.a);                      // a
                vzero(t0->accelerate.d);                      // d

                // Travel the remaning length
                double v0 = norm(t0->accelerate.v0);
                double len_t = len - len_d;
                t0->travel.t = len_t / v0;                     // t
                vcopy(t0->travel.p0, t0->accelerate.p1);       // p0
                vcopy(t0->travel.p1, t0->decelerate.p0);       // p1
                vcopy(t0->travel.v0, t0->accelerate.v0);       // v0
                vcopy(t0->travel.v1, t0->accelerate.v0);       // v1
                vzero(t0->travel.a);                           // a
                vsub(t0->travel.d, t0->travel.p1, t0->travel.p0);  // d
        }

        t0->accelerate.at = at;             
        t0->travel.at = t0->accelerate.at + t0->accelerate.t;
        t0->decelerate.at = t0->travel.at + t0->travel.t;
        t0->curve.at = t0->decelerate.at + t0->decelerate.t;
}

/**
 *  Readjusts the speed in case the next segment had to slow down.
 *
 */
static void segment_update_speeds(segment_t *s0, atdc_t *t0, double *amax)
{
        /* Compare the speeds at the start of the segments and at the
           entry of the curve. Check whether there is enough space to
           accelerate/decelerate. If not, adapt one of the two
           speeds. */

        /* The available path length */
        double len;
        double d[3];
        vsub(d, t0->curve.p0, t0->accelerate.p0);
        len = norm(d);

        /* The two speeds */
        double v0 = norm(t0->accelerate.v0);
        double v1 = norm(t0->curve.v1);
        double v0_next = 0.0;
        if (t0->next)
                v0_next = norm(t0->next->accelerate.v0);

        // The next segment lowered its entry speed. Slow down the
        // curve.
        if (v1 != v0_next) {
                double alpha = v0_next / v1;
                smul(t0->curve.v0, t0->curve.v0, alpha);
                smul(t0->curve.v1, t0->curve.v1, alpha);
                smul(t0->curve.a, t0->curve.a, alpha * alpha);
                t0->curve.t /= alpha;
        }
        
        /* The maximum allowable acceleration in the direction of this
           segment */
        double tmp[3];
        normalize(tmp, s0->section.d);
        vmul(tmp, tmp, amax);
        double am = norm(tmp);

        /* The path length needed to change the speed */
        double dv = v1 - v0;
        double dt = sign(dv) * dv / am;
        double dx = v0 * dt + 0.5 * am * dt * dt;

        if (dx > len && v0 > v1) {
                
                /* The speed at the entry of this segment is too high
                   and the segment is not long enough to slow down to
                   the maximum entry speed of the curve at the end of
                   the segment. We must scale back the speed at the
                   start of this segment. */

                /* This change will have to be propagated back to the
                   previous segments. This will be done during the
                   backward traversal of the segments. */
                
                dt = (sqrt(v1 * v1 + 2.0 * am * len) - v1) / am;
                double v0s = v1 + am * dt;
                double alpha = v0s / v0;
                smul(t0->accelerate.v0, t0->accelerate.v0, alpha);

                // Propagate the speed change backwards through the path.
                if (s0->prev == NULL || t0->prev == NULL) {
                        r_err("check_curve_speed_forward: pushing speed change "
                              "backwards but we're at the first segment?!");
                        return;
                }
                
                segment_update_speeds(s0->prev, t0->prev, amax);
                
        } else if (dx > len && v0 < v1) {
                
                /* The segment is too short to accelerate up to the
                   desired entry speed of the curve. Reduce the
                   curve's entry speed and recalculate the force to
                   apply.  TODO: ideally, the maximum acceleration is
                   maintained during the curve but the entry and exit
                   points are remapped. For simplicity, I started by
                   changing the acceleration and speeds. */
                
                dt = (sqrt(v0 * v0 + 2.0 * am * len) - v0) / am;
                double v1s = v0 + am * dt;
                double alpha = v1s / v1;
                smul(t0->curve.v0, t0->curve.v0, alpha);
                smul(t0->curve.v1, t0->curve.v1, alpha);
                smul(t0->curve.a, t0->curve.a, alpha * alpha);
                t0->curve.t /= alpha;
        }
}

/**
 *  Computes the curve at the end of the segment. This function may
 *  adjust the entry speed of the segment to assure that the curve
 *  isn't entered with too high a speed.
 *
 *  s0: The current segment.
 *  amax: The maximum acceleration that the machine tolerates. 
 */
static void segment_compute_curve_and_speeds(segment_t *s0, atdc_t *t0,
                                             double d, double *amax)
{
        segment_compute_curve(s0, t0, d, amax);
        segment_update_speeds(s0, t0, amax);
}

static void segment_check_max_speed(segment_t *s0, double *vmax)
{
        double s = 1.0;

        for (int i = 0; i < 3; i++)
                s = min(s, vmax[i] / fabs(s0->section.v0[i]));
        
        smul(s0->section.v0, s0->section.v0, s);
        smul(s0->section.v1, s0->section.v1, s);
}

/**************************************************************************/

static void compute_curves_and_speeds(segment_t *path, atdc_t *atdc,
                                      double d, double *amax)
{
        segment_t *s0 = path;
        atdc_t *t0 = atdc;
        
        while (s0) {
                segment_compute_curve_and_speeds(s0, t0, d, amax);
                s0 = s0->next;
                t0 = t0->next;
        }
}

static void compute_accelerations(segment_t *path, atdc_t *atdc, double *amax)
{
        segment_t *s0 = path;
        atdc_t *t0 = atdc;
        double at = 0.0;
        
        while (s0) {
                segment_compute_accelerations(s0, t0, amax, at);
                at = t0->curve.at + t0->curve.t;
                s0 = s0->next;
                t0 = t0->next;
        }
}

static void check_max_speeds(segment_t *path, double *vmax)
{
        for (segment_t *segment = path; segment != NULL; segment = segment->next)
                segment_check_max_speed(segment, vmax);
}

static atdc_t *copy_segments_to_atdc(segment_t *path)
{
        atdc_t *first = NULL;
        atdc_t *prev = NULL;
        
        for (segment_t *segment = path; segment != NULL; segment = segment->next) {
                atdc_t *atdc = new_atdc();
                
                // Append to double-linked list
                if (first == NULL)
                        first = atdc;
                if (prev != NULL) {
                        prev->next = atdc;
                        atdc->prev = prev;
                }
                prev = atdc;
                
                // Clone the list of actions
                for (list_t *l = segment->actions; l; l = list_next(l)) {
                        action_t *a = list_get(l, action_t);
                        atdc->actions = list_append(atdc->actions, action_clone(a));
                }
        }

        return first;
}

static atdc_t *segments_to_atdc(segment_t *path, double d, double *vmax, double *amax)
{
        check_max_speeds(path, vmax);
        atdc_t *atdc = copy_segments_to_atdc(path);

        compute_curves_and_speeds(path, atdc, d, amax);
                
        /* printf("Curves and speed:\n"); */
        /* atdc_print(atdc); */
        /* printf("\n\n"); */
        
        compute_accelerations(path, atdc, amax);
                
        /* printf("Accelerations:\n"); */
        /* atdc_print(atdc); */
        /* printf("\n\n"); */

        return atdc;
}

static list_t *segment_list_to_atdc_list(list_t *paths, double d, double *vmax, double *amax)
{
        list_t *atdc_list = NULL;
        
        for (list_t *l = paths; l != NULL; l = list_next(l)) {
                segment_t *path = list_get(l, segment_t);
                atdc_t *atdc = segments_to_atdc(path, d, vmax, amax);
                atdc_list = list_append(atdc_list, atdc);
        }
        return atdc_list;
}

static list_t *script_to_segment_list(script_t *script, double *pos)
{
        list_t *actions = script_actions(script);
        list_t *paths = NULL;
        segment_t *prev = NULL;
        segment_t *last = NULL;

        while (actions) {
                
                action_t *action = list_get(actions, action_t);
                
                if (action->type == ACTION_MOVE) {

                        /* If the displacement is less then 0.05 mm, skip it. */
                        double d[3];
                        vsub(d, action->p, pos); 
                        if (norm(d) < 0.00005) {
                                actions = list_next(actions);
                                continue;
                        }
                        
                        segment_t *segment = new_segment();
                        if (segment == NULL)
                                return NULL;

                        // chain the segment
                        segment->prev = prev;
                        if (prev) 
                                prev->next = segment;
                        else 
                                paths = list_append(paths, segment);
                        
                        prev = segment;
                        last = segment;
                        
                        // p0 and p1
                        vcopy(segment->section.p0, pos); 
                        vcopy(segment->section.p1, action->p); 

                        // d
                        vsub(segment->section.d,
                             segment->section.p1,
                             segment->section.p0); 
                        
                        // v0 and v1
                        double len = norm(segment->section.d);
                        normalize(segment->section.v0, segment->section.d);
                        smul(segment->section.v0, segment->section.v0, action->v);
                        vcopy(segment->section.v1, segment->section.v0);
                        
                        // a
                        vzero(segment->section.a);

                        // update current position
                        vcopy(pos, action->p); 
                        
                } else {

                        if (last == NULL) {
                                // Create a dummy segment to attach
                                // this action to.
                                last = new_segment();
                                if (last == NULL)
                                        return NULL;
                                paths = list_append(paths, last);
                        }

                        // Execute the action to the end of the current segment
                        last->actions = list_append(last->actions,
                                                    action_clone(action));

                        if (action->type == ACTION_DELAY
                           || action->type == ACTION_WAIT)
                                // Start a new path
                                prev = NULL;
                }
                
                actions = list_next(actions);
        }
        
        return paths;
}

/******************************************************************/

list_t *planner_slice(planner_t *planner, double period, double maxlen)
{
        list_t *list = NULL;
        for (list_t *l = planner->atdc_list; l != NULL; l = list_next(l)) {
                atdc_t *atdc = list_get(l, atdc_t);
                list_t *slices = atdc_list_slice(atdc, period, maxlen);
                if (slices == NULL) {
                        delete_section_list(list);
                        return NULL;
                }
                list = list_concat(list, slices);
        }
        return list;
}

static int planner_convert_script(planner_t *planner, script_t *script)
{
        int err = 0;
        double pos[3];
        
        if (controller_position(planner->controller, pos) != 0)
                return -1;

        planner->segments = script_to_segment_list(script, pos);
        if (planner->segments == NULL)
                return -1;
        
        planner->atdc_list = segment_list_to_atdc_list(planner->segments,
                                                       script_deviation(script),
                                                       planner->controller->vmax,
                                                       planner->controller->amax);
        if (planner->atdc_list == NULL)
                return -1;
        
        return 0;
}

list_t *planner_get_segments_list(planner_t *planner)
{
        return planner->segments;
}

list_t *planner_get_atdc_list(planner_t *planner)
{
        return planner->atdc_list;
}
        
