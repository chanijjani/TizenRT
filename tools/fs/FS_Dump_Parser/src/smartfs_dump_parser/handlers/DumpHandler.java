package smartfs_dump_parser.handlers;

import org.eclipse.e4.core.di.annotations.Execute;

import org.eclipse.swt.widgets.Shell;

public class DumpHandler {

	@Execute
	public void execute(Shell shell) {
		try {
			Thread pythonThread = new Thread(new PythonRunner());
			pythonThread.start();
			System.out.println("Python script for SmartFS dumping is executed!!\n");
		} catch (Exception e) {

		}
	}
}
