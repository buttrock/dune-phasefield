double evalRho(double x)
{
  {
    return(0.4958034541*x+0.1065766549);
  }
}


inline double helmholtz ( double rho ,double phi ) const
{
  double t1;
  double t16;
  double t17;
  double t18;
  double t2;
  double t20;
  double t22;
  double t29;
  double t5;
  {
    t1 = phi*phi;
    t2 = t1*t1;
    t5 = t1*phi;
    t16 = 6.0*t2*phi;
    t17 = 15.0*t2;
    t18 = 10.0*t5;
    t20 = rho*rho;
    t22 = t20*t20;
    t29 = log(rho);
    return(2.0*A_*(t2+(2.0*alpha_-2.0)*t5+(-3.0*alpha_+1.0)*t1+alpha_)/delta_+(t16-
t17+t18)*(0.8575855211E-1*t22*t20*rho-0.2868132559E-1*rho+0.1480890852E-1)+(1.0
-t16+t17-t18)*(0.3001504024E-1*rho*t29+0.3718535686E-1*rho+0.3198902536E-2));
  }
}


inline double reactionSource ( double rho ,double phi ) const
{
  double t1;
  double t17;
  double t2;
  double t21;
  double t22;
  double t24;
  double t31;
  {
    t1 = phi*phi;
    t2 = t1*phi;
    t17 = t1*t1;
    t21 = 30.0*t17-60.0*t2+30.0*t1;
    t22 = rho*rho;
    t24 = t22*t22;
    t31 = log(rho);
    return(2.0*A_*(4.0*t2+3.0*(2.0*alpha_-2.0)*t1+2.0*(-3.0*alpha_+1.0)*phi)/delta_
+t21*(0.8575855211E-1*t24*t22*rho-0.2868132559E-1*rho+0.1480890852E-1)-t21*(
0.3001504024E-1*rho*t31+0.3718535686E-1*rho+0.3198902536E-2));
  }
}


inline double dphireactionSource ( double rho ,double phi ) const
{
  double t1;
  double t17;
  double t18;
  double t20;
  double t27;
  {
    t1 = phi*phi;
    t17 = 120.0*t1*phi-180.0*t1+60.0*phi;
    t18 = rho*rho;
    t20 = t18*t18;
    t27 = log(rho);
    return(2.0*A_*(12.0*t1+6.0*(2.0*alpha_-2.0)*phi-6.0*alpha_+2.0)/delta_+t17*(
0.8575855211E-1*t20*t18*rho-0.2868132559E-1*rho+0.1480890852E-1)-t17*(
0.3001504024E-1*rho*t27+0.3718535686E-1*rho+0.3198902536E-2));
  }
}


inline double chemicalPotential ( double rho ,double phi ) const
{
  double t1;
  double t13;
  double t17;
  double t2;
  double t3;
  double t4;
  double t5;
  double t6;
  {
    t1 = phi*phi;
    t2 = t1*t1;
    t3 = t2*phi;
    t4 = rho*rho;
    t5 = t4*t4;
    t6 = t5*t4;
    t13 = t1*phi;
    t17 = log(rho);
    return(0.3601859189E1*t3*t6-0.5752903361*t3-0.9004647972E1*t2*t6+
0.143822584E1*t2+0.6003098648E1*t13*t6-0.9588172269*t13+0.3001504024E-1*t17+
0.672003971E-1-0.1800902414*t3*t17+0.4502256036*t2*t17-0.3001504024*t13*t17);
  }
}

inline double drhochemicalPotential ( double rho ,double phi ) const
{
  double t1;
  double t11;
  double t2;
  double t3;
  double t4;
  double t5;
  double t6;
  {
    t1 = phi*phi;
    t2 = t1*t1;
    t3 = t2*phi;
    t4 = rho*rho;
    t5 = t4*t4;
    t6 = t5*t4;
    t11 = t1*phi;
    return(0.4E-10*(0.5402788784E12*t3*t6-0.1350697196E13*t2*t6+0.9004647972E12
*t11*t6+750376006.0-4502256036.0*t3+0.1125564009E11*t2-7503760060.0*t11)/rho);
  }
}


inline double dphichemicalPotential ( double rho ,double phi ) const
{
  double t1;
  double t16;
  double t2;
  double t3;
  double t4;
  double t5;
  double t9;
  {
    t1 = phi*phi;
    t2 = t1*t1;
    t3 = rho*rho;
    t4 = t3*t3;
    t5 = t4*t3;
    t9 = t1*phi;
    t16 = log(rho);
    return(0.1800929594E2*t2*t5-0.2876451681E1*t2-0.3601859189E2*t9*t5+
0.5752903361E1*t9+0.1800929594E2*t1*t5-0.2876451681E1*t1-0.9004512072*t2*t16+
0.1800902414E1*t9*t16-0.9004512072*t1*t16);
  }
}


inline double pressure ( double rho ,double phi ) const
{
  double t1;
  double t11;
  double t2;
  double t22;
  double t4;
  double t5;
  double t6;
  double t7;
  double t9;
  {
    t1 = phi*phi;
    t2 = t1*t1;
    t4 = 6.0*t2*phi;
    t5 = 15.0*t2;
    t6 = t1*phi;
    t7 = 10.0*t6;
    t9 = rho*rho;
    t11 = t9*t9;
    t22 = log(rho);
    return((t4-t5+t7)*(-0.8575855211E-1*t11*t9*rho+0.2868132559E-1*rho
-0.1480890852E-1+rho*(0.6003098648*t11*t9-0.2868132559E-1))+(1.0-t4+t5-t7)*(
-0.3001504024E-1*rho*t22-0.3718535686E-1*rho-0.3198902536E-2+rho*(
0.3001504024E-1*t22+0.672003971E-1))-2.0*A_*(t2+(2.0*alpha_-2.0)*t6+(-3.0*alpha_+
1.0)*t1+alpha_)/delta_);
  }
}


inline double a ( double rho ,double phi ) const
{
  double t1;
  double t11;
  double t18;
  double t2;
  double t3;
  double t4;
  double t5;
  double t6;
  {
    t1 = phi*phi;
    t2 = t1*t1;
    t3 = t2*phi;
    t4 = rho*rho;
    t5 = t4*t4;
    t6 = t5*t4;
    t11 = t1*phi;
    t18 = sqrt(0.5402788782E13*t3*t6-0.1350697196E14*t2*t6+0.9004647972E13*t11*
t6+7503760060.0-0.4502256035E11*t3+0.1125564009E12*t2-0.750376006E11*t11);
    return(0.2E-5*t18);
  }
}

