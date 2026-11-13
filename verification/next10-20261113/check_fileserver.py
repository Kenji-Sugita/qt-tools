"""Check a separately built tutorial server with dummy files over loopback only."""
import argparse,json,os,socket,subprocess,tempfile,time
from pathlib import Path
parser=argparse.ArgumentParser();parser.add_argument('--build',type=Path,required=True);parser.add_argument('--report',type=Path,required=True);args=parser.parse_args();tests=[]
def check(name,condition):
 tests.append({'name':name,'passed':bool(condition)})
 if not condition:raise AssertionError(name)
build=args.build.resolve();env=dict(os.environ,LC_ALL='en_US.UTF-8')
with tempfile.TemporaryDirectory(prefix='fileserver-publication-') as temp:
 work=Path(temp);share=work/'share';share.mkdir();(share/'memo.txt').write_bytes(b'hello mcp\n');(share/'large.bin').write_bytes(b'x'*32);(share/'nested').mkdir();(share/'link.txt').symlink_to(share/'memo.txt')
 with socket.socket() as sock:sock.bind(('127.0.0.1',0));port=sock.getsockname()[1]
 server=build/'fileservermcp-server';client=build/'fileservermcp-client';url=f'http://127.0.0.1:{port}/sse'
 for executable in [server,client]:
  run=subprocess.run([str(executable),'--help'],env=env,capture_output=True,text=True,timeout=10);check(executable.name+' help',run.returncode==0 and 'Usage' in run.stdout)
 log=(work/'server.log').open('w');process=subprocess.Popen([str(server),'--root',str(share),'--port',str(port),'--max-size','16'],env=env,stdout=log,stderr=log)
 def call(*options,input=None):return subprocess.run([str(client),'--url',url,*options],env=env,input=input,capture_output=True,text=True,timeout=15)
 try:
  for _ in range(100):
   try:
    with socket.create_connection(('127.0.0.1',port),timeout=.1):break
   except OSError:time.sleep(.05)
  run=call('status');check('initialize and status',run.returncode==0 and 'Connected' in run.stdout)
  run=call('--json','tools');check('tools/list',run.returncode==0 and all(n in run.stdout for n in ['list_files','download_file']))
  run=call('--json','files');data=json.loads(run.stdout);check('files exclude symlinks and subdirectories',run.returncode==0 and {f['name'] for f in data['files']}=={'memo.txt','large.bin'})
  output=work/'download.txt';run=call('download','memo.txt',str(output));check('download exact bytes',run.returncode==0 and output.read_bytes()==b'hello mcp\n')
  run=call('download','memo.txt',str(output));check('refuse overwrite',run.returncode!=0 and output.read_bytes()==b'hello mcp\n')
  for name,error in [('missing.txt','file not found'),('../outside.txt','invalid file name'),('link.txt','file not found'),('large.bin','file is larger than max-size')]:
   run=call('download',name,str(work/'rejected.txt'));check('reject '+name,run.returncode!=0 and error in run.stdout+run.stderr)
  run=call('call','list_files','{}');check('direct tools/call',run.returncode==0 and 'memo.txt' in run.stdout)
  run=call(input='.files\n.q\n');check('REPL list and quit',run.returncode==0 and 'memo.txt' in run.stdout)
 finally:process.terminate();process.wait(timeout=10);log.close();args.report.write_text(json.dumps({'qt':'6.12.0','scope':'macOS arm64, dummy files, loopback; no current Qt Creator connection test','tests':tests},indent=2)+'\n')
print(len(tests),'checks passed')
