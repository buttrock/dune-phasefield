restart; with(codegen); outstring := "maple.cc"; outstring2 := "balancedh.cc";
F11 := proc (rho) options operator, arrow; (b-c)*rho+c*rho*ln(rho)+d end proc; g1 := D(F11);
F00 := proc (rho) options operator, arrow; (f-e)*rho+e*rho*ln(rho)+g end proc; g0 := D(F00);
c := 1.5; b := log(2); d := 0; e := 1; f := 0; z1 := solve(g1(rho) = 0, rho); z2 := solve(g0(rho) = 0.*rho); gg := F1(z1)-e*z2*ln(z2)-(f-e)*z2; g := .5;
G1 := D(F11); G0 := D(F00); p1 := proc (rho) options operator, arrow; -F11(rho)+rho*G1(rho) end proc; p0 := proc (rho) options operator, arrow; -F00(rho)+rho*G0(rho) end proc;
sols := fsolve([p1(rho) = p0(x), G1(rho) = G0(x)], {rho = 0 .. 4, x = 0 .. 4}); sol1 := solve(sols[1]); sol2 := solve(sols[2]); pconst := 1/2*(p1(sol1)+p0(sol2)); mwg := proc (rho) options operator, arrow; G1(sol1)*rho-p1(sol1) end proc; F1 := F11-mwg; F0 := F00-mwg;
nu := proc (phi) options operator, arrow; 6*phi^5-15*phi^4+10*phi^3 end proc; W := proc (phi) options operator, arrow; 2*A*(phi^4+(2*alpha-2)*phi^3+(-3*alpha+1)*phi^2+alpha) end proc; dW := D(W); F := proc (rho, phi) options operator, arrow; rho*W(phi)/delta+beta*(nu(phi)*F1(rho)+(1-nu(phi))*F0(rho)) end proc; Pressure := proc (rho, phi) options operator, arrow; beta*(nu(phi)*p1(rho)+(1-nu(phi))*p0(rho)) end proc; Potential := proc (rho, phi) options operator, arrow; W(phi)/delta+beta*(nu(phi)*G1(rho)+(1-nu(phi))*G0(rho)) end proc;
S := proc (rho, phi) options operator, arrow; diff(F(rho, phi), phi) end proc;
solrho := proc (x) options operator, arrow; (Const+nu(x)*(d-g)+g)/(nu(x)*(c-e)+e) end proc; Const := pconst; solproc := makeproc(solrho(x), x); solproc1 := optimize(solproc); C(solproc1, filename = outstring, ansi);
Fproc := makeproc(F(rho, phi), [rho, phi]); helmholtz := optimize(Fproc); C(helmholtz, filename = outstring, ansi);
S := proc (rho, phi) options operator, arrow; diff(F(rho, phi), phi) end proc; Sproc := makeproc(S(rho, phi), [rho, phi]); reactionSource := optimize(Sproc); C(reactionSource, filename = outstring, ansi);

dphiS := proc (rho, phi) options operator, arrow; diff(S(rho, phi), phi) end proc; dphiSproc := makeproc(dphiS(rho, phi), [rho, phi]); dphireactionSource := optimize(dphiSproc); C(dphireactionSource, filename = outstring, ansi);
CProc := makeproc(Potential(rho, phi), [rho, phi]); chemicalPotential := optimize(CProc); C(chemicalPotential, filename = outstring, ansi);
drhoPotential := proc (rho, phi) options operator, arrow; diff(Potential(rho, phi), rho) end proc; drhomuproc := makeproc(simplify(drhoPotential(rho, phi)), [rho, phi]); drhochemicalPotential := optimize(drhomuproc); C(drhochemicalPotential, filename = outstring, ansi);
dphiPotential := proc (rho, phi) options operator, arrow; diff(Potential(rho, phi), phi) end proc; dphimuproc := makeproc(simplify(dphiPotential(rho, phi)), [rho, phi]); dphichemicalPotential := optimize(dphimuproc); C(dphichemicalPotential, filename = outstring, ansi);
Pproc := makeproc(simplify(Pressure(rho, phi)), [rho, phi]); pressure := optimize(Pproc); C(pressure, filename = outstring, ansi);
wavespeed := proc (rho, phi) options operator, arrow; sqrt(diff(Pressure(rho, phi), rho)) end proc; wproc := makeproc(simplify(wavespeed(rho, phi)), [rho, phi]); a := optimize(wproc); C(a, filename = outstring, ansi);

solphi := proc (x) options operator, arrow; .5*tanh(x/delta)+.5 end proc; rs := proc (x) options operator, arrow; solrho(solphi(x)) end proc; rhosol := makeproc(rs(x), x); rhosol := optimize(rhosol); gr := proc (x) options operator, arrow; diff(rhosol(x), x) end proc; gradrho := makeproc(gr(x), x); gradrho := optimize(gradrho); C(rhosol, filename = outstring2, ansi); C(gradrho, filename = outstring2, ansi);
gp := D(solphi); gradphi := makeproc(gp(x), x); C(gradphi, filename = outstring2, ansi);
lp := D(gradphi); laplacephi := makeproc(lp(x), x);
tht := proc (x) options operator, arrow; reactionSource(rhosol(x), solphi(x))-delta*laplacephi(x)/rhosol(x)+delta*gradrho(x)*gradphi(x)/(rhosol(x)*rhosol(x)) end proc; thetasol := makeproc(tht(x), x); thetasol := optimize(thetasol); C(thetasol, filename = outstring2, ansi);

RsP := proc (x) options operator, arrow; (reactionSource(rhosol(x), solphi(x))-delta*laplacephi(x)/rhosol(x)+delta*gradrho(x)*gradphi(x)/(rhosol(x)*rhosol(x)))/rhosol(x) end proc; phiSource := makeproc(RsP(x), x); phiSource := optimize(phiSource); C(phiSource, filename = outstring2, ansi);
ms := proc (x) options operator, arrow; chemicalPotential(rhosol(x), solphi(x))+(-1/2)*delta*gradphi(x)*gradphi(x)/(rhosol(x)*rhosol(x)) end proc;
musol := makeproc(ms(x), x); musol := optimize(musol); C(musol, filename = outstring2, ansi);
dsolmu := proc (x) options operator, arrow; diff(musol(x), x) end proc; gradmu := makeproc(dsolmu(x), x); gradmu := optimize(gradmu);
RsV := proc (x) options operator, arrow; -gradphi(x)*thetasol(x)+rhosol(x)*gradmu(x) end proc;
veloSource := makeproc(RsV(x), x); veloSource := optimize(veloSource); C(veloSource, filename = outstring2, ansi);
# 
# 
