restart; with(codegen);
F1 := proc (rho) options operator, arrow; (b-a)*rho+a*rho*ln(rho)+d end proc; g1 := D(F1); simplify(-F1(rho)+rho*g1(rho));
F0 := proc (rho) options operator, arrow; (f-e)*rho+e*rho*ln(rho)+g end proc; g0 := D(F0); solve(g0(rho) = 0, rho); simplify(-F0(rho)+rho*g0(rho));
a := 1.5; b := -1; d := 1; e := 3; f := -4; z1 := solve(g1(rho) = 0, rho); z2 := solve(g0(rho) = 0.*rho); gg := F1(z1)-e*z2*ln(z2)-(f-e)*z2; g := gg; F1(z1); F0(z2);
G1 := D(F1); G0 := D(F0); p1 := proc (rho) options operator, arrow; -F1(rho)+rho*G1(rho) end proc; p0 := proc (rho) options operator, arrow; -F0(rho)+rho*G0(rho) end proc;
fsolve([p1(rho) = p0(x), G1(rho) = G0(x)], {rho = 0 .. 4, x = 0 .. 4}); pconst := 1/2*(p1(1.947734046)+p0(3.793667900));
nu := proc (phi) options operator, arrow; 6*phi^5-15*phi^4+10*phi^3 end proc; W := proc (phi) options operator, arrow; 2*phi^2*(1-phi)^2 end proc; dW := D(W); F := proc (rho, phi) options operator, arrow; rho*W(phi)/delta+nu(phi)*F1(rho)+(1-nu(phi))*F0(rho) end proc; Pressure := proc (rho, phi) options operator, arrow; nu(phi)*p1(rho)+(1-nu(phi))*p0(rho) end proc; Potential := proc (rho, phi) options operator, arrow; W(phi)/delta+nu(phi)*G1(rho)+(1-nu(phi))*G0(rho) end proc;
S := proc (rho, phi) options operator, arrow; diff(F(rho, phi), phi) end proc; outstring := "maple.c";
solrho := proc (x) options operator, arrow; (Const+nu(x)*(d-g)+g)/(nu(x)*(a-e)+e) end proc; Const := pconst; solproc := makeproc(solrho(x), x); solproc1 := optimize(solproc); C(solproc1, filename = outstring, ansi);
Fproc := makeproc(F(rho, phi), [rho, phi]); helmholtz := optimize(Fproc); C(helmholtz, filename = outstring, ansi);
S := proc (rho, phi) options operator, arrow; diff(F(rho, phi), phi) end proc; Sproc := makeproc(S(rho, phi), [rho, phi]); reactionSource := optimize(Sproc); C(reactionSource, filename = outstring, ansi);
dphiS := proc (rho, phi) options operator, arrow; diff(S(rho, phi), phi) end proc; dphiSproc := makeproc(dphiS(rho, phi), [rho, phi]); dphireactionSource := optimize(dphiSproc); C(dphireactionSource, filename = outstring, ansi);
CProc := makeproc(Potential(rho, phi), [rho, phi]); chemicalPotential := optimize(CProc); C(chemicalPotential, filename = outstring, ansi);
drhoPotential := proc (rho, phi) options operator, arrow; diff(Potential(rho, phi), rho) end proc; drhomuproc := makeproc(simplify(drhoPotential(rho, phi)), [rho, phi]); drhochemicalPotential := optimize(drhomuproc); C(drhochemicalPotential, filename = outstring, ansi);
dphiPotential := proc (rho, phi) options operator, arrow; diff(Potential(rho, phi), phi) end proc; dphimuproc := makeproc(simplify(dphiPotential(rho, phi)), [rho, phi]); dphichemicalPotential := optimize(dphimuproc); C(dphichemicalPotential, filename = outstring, ansi);
Pproc := makeproc(simplify(Pressure(rho, phi)), [rho, phi]); pressure := optimize(Pproc); C(pressure, filename = outstring, ansi);
wavespeed := proc (rho, phi) options operator, arrow; sqrt(diff(Pressure(rho, phi), rho)) end proc; wproc := makeproc(simplify(wavespeed(rho, phi)), [rho, phi]); a := optimize(wproc); C(a, filename = outstring, ansi);

# 
# 