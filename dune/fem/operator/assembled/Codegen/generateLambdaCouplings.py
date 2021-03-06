import os, subprocess, sys
rho='rho'
v='v'
vx='vx'
vy='vy'
u='u'
phi='phi'
mu='mu'
tau='tau'
lambdax='lambdax'
lambday='lambda'
sigmax='sigmax'
sigmay='sigmay'


variables1d={ rho: 0 , v: 1 ,phi: 2, mu : 3 , tau : 4, sigmax:  5, lambdax: 6 }
varlist=list(variables1d.keys())
varlist.sort()

elemcouplings1d=[ ( rho , rho ), ( rho , v ), 
            ( v , rho ) ,( v , v), ( v, phi ),( v , mu ),( v , tau ),
            ( phi , rho ),( phi , v ), ( phi,phi),(phi,tau),
            ( mu, rho),(mu,v),(mu,phi),(mu,mu),(mu,sigmax),
            ( tau,rho),(tau,phi),(tau,tau),(tau,lambdax),
            (sigmax,phi),(sigmax,sigmax),
            (lambdax,rho),(lambdax,sigmax),(lambdax,lambdax)
]
print(len( elemcouplings1d))
interseccouplings1d=[  ( rho , rho ), ( rho , v ),( rho , mu),
            ( v , rho ) ,( v , v), ( v, phi ),( v , mu ),( v , tau ),
            ( phi , v ), ( phi,phi),
            (tau,phi),(tau,lambdax),
            (sigmax,phi)
]
f=open('elementCouplings1d_lambda_CODEGEN.c','w')
i=0
for cp in elemcouplings1d:
  v1=variables1d[ cp[0] ]
  v2=variables1d[ cp[1] ]
  newline='elementCouplings_[ '+str( i )+' ] = std::make_pair('+str(v1)+','+str(v2)+');//('+cp[0]+','+cp[1]+')\n'
  f.write(newline)
  i=i+1
f.close()
f=open('intersectionCouplings1d_lambda_CODEGEN.c','w')
i=0
for cp in interseccouplings1d:
  v1=variables1d[ cp[0] ]
  v2=variables1d[ cp[1] ]
  newline='intersectionCouplings_[ '+str( i )+' ] = std::make_pair('+str(v1)+','+str(v2)+');//('+cp[0]+','+cp[1]+')\n'
  f.write(newline)
  i=i+1
f.close()

variables2d={ rho: 0 , vx: 1 , vy:2 ,phi: 3, mu : 4 , tau : 5, sigmax:  6 ,sigmay: 7,lambdax:8,lambday:9}
varlist=list(variables2d.keys())
varlist.sort()

elemcouplings2d=[ ( rho , rho ),( rho , vx ),(rho,vy),
            ( vx , rho ),( vx , vx ),( vx , vy ), ( vx, phi ),( vx , mu ),( vx , tau ),
            ( vy , rho ),( vy , vx ),( vy , vy ), ( vy, phi ),( vy , mu ),( vy , tau ),
            ( phi , rho ),( phi , vx ),( phi , vy ), ( phi,phi),(phi,tau),
            ( mu, rho ),(mu,vx),(mu,vy),(mu,phi),(mu,mu),(mu,sigmax),(mu,sigmay),
            (tau,rho),(tau,phi),(tau,tau),(tau,lambdax),(tau,lambday),
            (sigmax,phi),(sigmax,sigmax),
            (sigmay,phi),(sigmay,sigmay),
            (lambdax,rho),(lambdax,sigmax),(lambdax,lambdax),
            (lambday,rho),(lambday,sigmay),(lambday,lambday),
]
interseccouplings2d=[  ( rho , rho ), ( rho , vx ),( rho , vy ),( rho , mu),
            ( vx , rho ),( vx , vx ),(vx,vy), ( vx, phi ),( vx , mu ),( vx , tau ),
            ( vy , rho ),( vy , vx ),(vy,vy), ( vy, phi ),( vy , mu ),( vy , tau ),
            ( phi , vx ),( phi , vy ), ( phi,phi),
            (tau,phi),(tau,lambdax),(tau,lambday),
            (sigmax,phi),
            (sigmay,phi)
]
f=open('elementCouplings2d_lambda_CODEGEN.c','w')
i=0
for cp in elemcouplings2d:
  v1=variables2d[ cp[0] ]
  v2=variables2d[ cp[1] ]
  newline='elementCouplings_[ '+str( i )+' ] = std::make_pair('+str(v1)+','+str(v2)+');//('+cp[0]+','+cp[1]+')\n'
  f.write(newline)
  i=i+1
f.close()
f=open('intersectionCouplings2d_lambda_CODEGEN.c','w')
i=0
for cp in interseccouplings2d:
  v1=variables2d[ cp[0] ]
  v2=variables2d[ cp[1] ]
  newline='intersectionCouplings_[ '+str( i )+' ] = std::make_pair('+str(v1)+','+str(v2)+');//('+cp[0]+','+cp[1]+')\n'
  f.write(newline)
  i=i+1
f.close()

