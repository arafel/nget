#    util.py - nget test utility stuff
#    Copyright (C) 2002  Matthew Mueller <donut@azstarnet.com>
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import os, random, sys, shutil

def exitstatus(st):
	if not hasattr(os, 'WEXITSTATUS'): #the os.W* funcs are only on *ix
		return st
	if os.WIFSTOPPED(st):
		return 'stopped',os.WSTOPSIG(st)
	if os.WIFSIGNALED(st):
		return 'signal',os.WTERMSIG(st)
	if os.WIFEXITED(st):
		return os.WEXITSTATUS(st)
	return 'unknown',st

def vsystem(cmd):
	print 'running %r'%cmd
	return exitstatus(os.system(cmd))
	
class TestNGet:
	def __init__(self, nget, servers, priorities=None, options=None, hostoptions=None):
		defaultoptions = {
#			'tries': 1,
			'tries': 2,
			'delay': 0,
#			'penaltystrikes': 1,
#			'debug': 3,
#			'debug': 2,
#			'fullxover': 1
		}
		self.exe = nget
		self.rcdir = os.path.join(os.environ.get('TMPDIR') or '/tmp', 'nget_test_'+hex(random.randrange(0,sys.maxint)))
		os.mkdir(self.rcdir)
		self.tmpdir = os.path.join(self.rcdir, 'tmp')
		os.mkdir(self.tmpdir)

		rc = open(os.path.join(self.rcdir, '_ngetrc'), 'w')
		if options:
			defaultoptions.update(options)
		for k,v in defaultoptions.items():
			rc.write("%s=%s\n"%(k,v))

		rc.write("{halias\n")
		for i in range(0, len(servers)):
			rc.write("""
 {host%i
  addr=%s
  shortname=h%i
  id=%i
"""%(i, ':'.join(map(str,servers[i].socket.getsockname())), i, i+1))
			if hostoptions:
				opts = hostoptions[i]
				for k,v in opts.items():
					rc.write("%s=%s\n"%(k,v))
			rc.write(" }\n")
		rc.write("}\n")

		if priorities:
			rc.write("{hpriority\n")
			rc.write(" {default\n")
			for i,p in zip(range(0, len(servers)), priorities):
				rc.write("  host%i=%s\n"%(i, p))
			rc.write(" }\n")
			rc.write("}\n")
			
		rc.close()

		#print 'begin ngetrc:\n',open(os.path.join(self.rcdir, '_ngetrc'), 'r').read()
		#print '--end ngetrc'
	
	def run(self, args, pre=""):
		os.environ['NGETHOME'] = self.rcdir
		return vsystem(pre + self.exe+" -p "+self.tmpdir+" "+args)
	
	def run_getoutput(self, args, pre=""):
		outputpath = os.path.join(self.rcdir,'output.'+hex(random.randrange(0,sys.maxint)))
		status = self.run(args+' > '+outputpath, pre)
		f = open(outputpath, "r")
		output = f.read()
		f.close()
		return status, output
	
	def runlite(self, args, pre=""):
		olddir = os.getcwd()
		try:
			ngetliteexe = os.path.join(os.path.split(self.exe)[0], 'ngetlite')
			os.chdir(self.tmpdir)
			return vsystem(pre + ngetliteexe+" "+args)
		finally:
			os.chdir(olddir)

	
	def clean_tmp(self):
		shutil.rmtree(self.tmpdir)
		os.mkdir(self.tmpdir)
		
	def clean_all(self):
		if os.environ.get('TEST_NGET_NOCLEAN'):
			return
		shutil.rmtree(self.rcdir)
		

