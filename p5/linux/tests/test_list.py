tester_files = ["shutdown", "creat", "write", "dirs", "extracreds"]

def test_list(project_path, test_path, log):
   l = list()
   for f in tester_files:
      module = __import__(f)
      l.extend(module.test_list)
   return l
