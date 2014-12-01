#! /usr/bin/env python
import subprocess,sys,pickle,time
from Tkinter import *
terminal="xfce4-terminal"

#==================================
#the Runner class
#==================================
class PhasefieldDeltaRunner:
  def __init__(self, master):
    listframe=Frame(master)
    listframe.pack()
    paramframe=Frame(master)
    paramframe.pack()
    entryframe=Frame(master)
    entryframe.pack()
    messageframe=Frame(master)
    messageframe.pack(side=BOTTOM)
    self.programms=pickle.load(open("compiled.p","rb"))
    self.params=['phasefield.delta','phasefield.mu1','phasefield.mu2','phasefield.endTime', 
    'fem.prefix','phasefield.alpha','phasefield.beta','phasefield.A',
    'phasefield.rhofactor','fem.ode.odesolver','phasefield.acpenalty']
    self.defaults=['0.1','0.1','0.1','10','','0.','1.','0.','3.5','IM',0]
    self.paramEntries=self.makeform(paramframe,self.params)
    self.listbox = Listbox( listframe )
    self.listbox.pack(side=LEFT)
    for item in self.programms:
      self.listbox.insert(END, item)
    self.make=Button( entryframe, text="Run", command=self.runit)
    self.make.pack(side=RIGHT)
    self.scrvar=IntVar()
    scr=Radiobutton(entryframe,text='screen',variable=self.scrvar,value=1).pack() 
    scr=Radiobutton(entryframe,text='show',variable=self.scrvar,value=0).pack() 
  def makeform( self , root ,fields):
    entries={}
    count=0
    for field in fields:
      row=Frame(root)
      lab=Label(row,text=field)
      ent=Entry(row)
      row.pack(side=TOP,fill=X)
      lab.pack(side=LEFT)
      ent.pack(side=RIGHT,fill=X,expand=YES)
      ent.insert(0,self.defaults[count])
      entries.update({field : ent})
      count+=1
    return entries
  def composeString( self,mydelta,reac,start,printCount,step ):
    loopstring=' phasefield.delta:'+str(mydelta)+' phasefield.reactionrate:'+str(reac)+' phasefield.startLevel:'+str(start)+' phasefield.printCount:'+str(printCount)+' fixedTimeStep:'+str(step)+' '
    myday=time.strftime("%d_%m_%Y") 
    idxs = self.listbox.curselection()
    index=int(idxs[0])
    p=self.programms[index]
    outfile='./Data'+'_deltarun/'+self.paramEntries['fem.prefix'].get()+'_'+str(mydelta)
    paramstring=''
    for key in self.paramEntries:
      if key =='fem.prefix':
        paramstring+=key+':'+outfile+'_'+p+'_'+str(mydelta)+' '
      elif key == 'phasefield.delta':
        paramstring+=''
      else:
        paramstring+=key+':'+self.paramEntries[key].get()+' '
    execstring ='./'+p+loopstring+' '+paramstring+'paramfile:parameter_gui'
    #execstring ='./'+p+' '+paramstring+'paramfile:parameter_gui &>'+p+'.out'
    return ( execstring , outfile )
  def runcall(self, stringtuple):
    execstring=stringtuple[0]
    print(execstring)
    outfile=stringtuple[1]
    print( outfile )
    print( self.scrvar.get()) 
    if self.scrvar.get() == 0: 
      c = subprocess.call([execstring], shell=True)
    else:
      print('call screen ')
      c = subprocess.call([terminal+' -e "screen '+execstring+'"'], shell=True)
      subprocess.call(['screen -d'],shell=True)
      print('detached!!!')
  def runit(self):
    mydelta=float(self.paramEntries['phasefield.delta'].get())
    reac=1./mydelta
    start=1
    printCount=100
    step=5e-4
    runs=3
    while runs>0: 
      runstring=self.composeString(mydelta,reac,start,printCount,step) 
      self.runcall(runstring)
      mydelta=0.5*mydelta
      reac=reac*2
      start=start+1
      printCount=2*printCount
      step=0.25*step
      runs=runs-1

