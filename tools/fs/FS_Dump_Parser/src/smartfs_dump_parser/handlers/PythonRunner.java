package smartfs_dump_parser.handlers;

import java.io.File;

public class PythonRunner implements Runnable {

	private final String pythonScriptName = "dump_board_log.py";

	public void run() {
		try {
			File file = new File(this.getClass().getProtectionDomain().getCodeSource().getLocation().toURI());
			String pythonScriptPath = file.getCanonicalPath() + "/python/";
			String params[] = { "python", pythonScriptPath + pythonScriptName, "/dev/ttyACM0" };
			ProcessBuilder builder = new ProcessBuilder(params);
//			builder.redirectOutput(Redirect.INHERIT);		// Print out to console.
			builder.redirectOutput(new File(pythonScriptPath, "Dump_File.txt"));
			builder.start();
		} catch (Exception e) {

		}
	}
}
