/// <reference types="emscripten" />

declare namespace mGBA {
  /**
   * Common filesystem paths for the embedded virtual file system (IDBFS).
   *
   * These are the mount points/directories created by `FSInit` (ex. `/data/*`, `/autosave`, etc),
   * these are needed for building paths when you’re reading/writing files via `Module.FS`.
   */
  export interface filePaths {
    root: string;
    cheatsPath: string;
    gamePath: string;
    savePath: string;
    saveStatePath: string;
    screenshotsPath: string;
    patchPath: string;
    autosave: string;
  }

  export type coreCallbacks = {
    /**
     * Callback fired when the emulation core has triggered an alarm.
     */
    alarmCallback?: (() => void) | null;

    /**
     * Callback fired when the emulation core has crashed.
     */
    coreCrashedCallback?: (() => void) | null;

    /**
     * Callback fired when the emulation core has read input keys.
     */
    keysReadCallback?: (() => void) | null;

    /**
     * Callback fired when the emulation core has updated its save data.
     */
    saveDataUpdatedCallback?: (() => void) | null;

    /**
     * Callback fired when the emulation core has finished rendering a video frame.
     */
    videoFrameEndedCallback?: (() => void) | null;

    /**
     * Callback fired when the emulation core has started rendering a video frame.
     */
    videoFrameStartedCallback?: (() => void) | null;

    /**
     * Callback fired when the emulation core has captured an auto save state.
     */
    autoSaveStateCapturedCallback?: (() => void) | null;

    /**
     * Callback fired when the emulation core has finished loading an auto save state.
     */
    autoSaveStateLoadedCallback?: (() => void) | null;
  };

  export type coreSettings = {
    /**
     * Number of frames to skip rendering between screen paints.
     *
     * Typical values: `0..10`
     *
     * Default: 0
     */
    frameSkip?: number;

    /**
     * Target base frames-per-second for the emulation core. Used by timing
     * and frame-rate calculations.
     *
     * Typical values: 60, 30
     *
     * Default: 60
     */
    baseFpsTarget?: number;

    /**
     * Maximum number of rewind states to keep in memory. Larger values allow
     * longer rewind history at the cost of consumed memory. Value is a count of
     * historical entries in the buffer.
     *
     * Typical values: `100..10000` is reasonable depending on memory pressure and
     * the rewind interval.
     *
     * Default: 600
     */
    rewindBufferCapacity?: number;

    /**
     * The speed at which rewind snapshots are taken. Larger numbers mean rewind happens faster.
     *
     * Typical values: `1..10`
     *
     * Default: 1
     */
    rewindBufferInterval?: number;

    /**
     * Requested audio sample rate in Hz for the audio output.
     * The core will attempt to use this rate, actual output depends on
     * the host audio device (best effort).
     *
     * Typical values: 22050, 32000, 44100, 48000
     *
     * Default: 48000
     */
    audioSampleRate?: number;

    /**
     * Preferred size, in samples, of the audio buffer. Smaller buffers reduce
     * latency but increase the chance of underruns, larger buffers increase
     * latency but are more stable.
     *
     * Typical values: `256..4096`
     *
     * Default: 1024
     */
    audioBufferSize?: number;

    /**
     * Interval in seconds between periodic auto save state captures.
     *
     * Typical values: `10..30`
     *
     * Default: 30
     */
    autoSaveStateTimerIntervalSeconds?: number;

    /**
     * If true, allows opposing directional inputs (ex. left + right) to be
     * accepted simultaneously. When false, only a single directional input
     * is accepted at a time.
     *
     * Default: true
     */
    allowOpposingDirections?: boolean;

    /**
     * If true, synchronize the video frame rate to the host display refresh rate (vsync).
     *
     * Default: false
     */
    videoSync?: boolean;

    /**
     * If true, synchronizes the video frame rate to the audio output speed.
     *
     * Default: false
     */
    audioSync?: boolean;

    /**
     * If true, render video on a separate thread (if supported).
     *
     * Default: false
     */
    threadedVideo?: boolean;

    /**
     * Enable/disable rewind support.
     *
     * Default: true
     */
    rewindEnable?: boolean;

    /**
     * If true, the core will sync using discrete timestep increments based on
     * the baseFpsTarget value rather than variable-step delta timing (audio/video).
     *
     * Default: true
     */
    timestepSync?: boolean;

    /**
     * If true, show an on-screen true FPS counter overlay.
     *
     * Default: false
     */
    showFpsCounter?: boolean;

    /**
     * Enable/disable periodic auto save state capture.
     *
     * Default: true
     */
    autoSaveStateEnable?: boolean;

    /**
     * If true, attempts to load the latest auto save state after a game load/reset.
     *
     * Default: true
     */
    restoreAutoSaveStateOnLoad?: boolean;
  };

  export interface mGBAEmulator extends EmscriptenModule {
    // custom methods from preamble
    /**
     * Attempts to automatically load cheats for the currently loaded game.
     *
     * @returns True if cheats were loaded successfully, otherwise false.
     */
    autoLoadCheats(): boolean;

    /**
     * Binds a physical key (SDL key name) to a GBA input action.
     *
     * @param bindingName - SDL key name (ex. "Return", "Left", "Keypad X").
     * @param inputName - GBA input/action (ex. "A", "B", "Start", "Up").
     */
    bindKey(bindingName: string, inputName: string): void;

    /**
     * Presses a named emulated button.
     *
     * @param name - Button name to press (ex. "A", "B", "Start", "Select", "Up").
     */
    buttonPress(name: string): void;

    /**
     * Releases (unpress) a named emulated button.
     *
     * @param name - Button name to release (ex. "A", "B", "Start", "Select", "Up").
     */
    buttonUnpress(name: string): void;

    /**
     * Initializes the Emscripten filesystem and any runtime FS mounts needed by the emulator.
     */
    FSInit(): Promise<void>;

    /**
     * Synchronizes filesystem state to the backing store (IDBFS).
     */
    FSSync(): Promise<void>;

    /**
     * Gets the current fast forward multiplier.
     *
     * - `multiplier = 1` = normal speed
     * - `multiplier > 1` = fast forward (`xN`)
     * - `multiplier < 0` = slow down (`1/abs(N)`)
     *
     * @returns Current fast forward multiplier.
     */
    getFastForwardMultiplier(): number;

    /**
     * Gets the current main loop timing mode.
     *
     * @returns Current timing mode value.
     */
    getMainLoopTimingMode(): number;

    /**
     * Gets the current main loop timing value for the current mode.
     *
     * @returns Current timing value.
     */
    getMainLoopTimingValue(): number;

    /**
     * Reads the currently loaded game save data.
     *
     * @returns Save data as bytes, or null if no save exists.
     */
    getSave(): Uint8Array | null;

    /**
     * Gets the current audio volume multiplier.
     *
     * - `1.0` = 100%
     * - Range is `0.0..2.0` (0%..200%)
     *
     * @returns Current volume multiplier.
     */
    getVolume(): number;

    /**
     * Lists ROM file names under the game directory.
     *
     * @returns Directory entries from `filePaths().gamePath`.
     */
    listRoms(): string[];

    /**
     * Lists save file names under the save directory.
     *
     * @returns Directory entries from `filePaths().savePath`.
     */
    listSaves(): string[];

    /**
     * Loads a rom into the emulator and runs the game, optionally overriding the save path used.
     *
     * @param romPath - Path to the rom file within the emulated filesystem.
     * @param savePathOverride - Optional override for the save path.
     * @returns True if the game loaded successfully, otherwise false.
     */
    loadGame(romPath: string, savePathOverride?: string): boolean;

    /**
     * Loads a save state from a given slot.
     *
     * @param slot - State slot index.
     * @returns True if the state loaded successfully, otherwise false.
     */
    loadState(slot: number): boolean;

    /**
     * Forces an auto save state capture immediately.
     *
     * @returns True if the auto save state was captured successfully, otherwise false.
     */
    forceAutoSaveState(): boolean;

    /**
     * Loads the current auto save state (if present).
     *
     * @returns True if the auto save state loaded successfully, otherwise false.
     */
    loadAutoSaveState(): boolean;

    /**
     * Reads the current auto save state from the filesystem (if present).
     *
     * @returns The auto save state name and bytes, or null if none exists.
     */
    getAutoSaveState(): { autoSaveStateName: string; data: Uint8Array } | null;

    /**
     * Uploads auto save state data into the emulator filesystem.
     *
     * @param autoSaveStateName - Full auto save path to write to.
     * @param data - Raw auto save state bytes.
     */
    uploadAutoSaveState(
      autoSaveStateName: string,
      data: Uint8Array,
    ): Promise<void>;

    /**
     * Pauses audio output without pausing emulation.
     */
    pauseAudio(): void;

    /**
     * Pauses the game emulation.
     */
    pauseGame(): void;

    /**
     * Performs a quick reload of the current ROM file.
     */
    quickReload(): void;

    /**
     * Quits the currently running game session.
     */
    quitGame(): void;

    /**
     * Exits the emulator runtime.
     */
    quitMgba(): void;

    /**
     * Resumes audio output.
     */
    resumeAudio(): void;

    /**
     * Resumes game emulation and audio.
     */
    resumeGame(): void;

    /**
     * Saves a save state to a given slot.
     *
     * @param slot - State slot index.
     * @returns True if the state saved successfully, otherwise false.
     */
    saveState(slot: number): boolean;

    /**
     * Captures a screenshot and writes it to the screenshots directory.
     *
     * @param fileName - Optional custom screenshot file name.
     * @returns True if screenshot was captured successfully, otherwise false.
     */
    screenshot(fileName?: string): boolean;

    /**
     * Sets the current fast forward multiplier.
     *
     * - `multiplier = 1` = normal speed
     * - `multiplier > 1` = fast forward (`xN`)
     * - `multiplier < 0` = slow down (`1/abs(N)`)
     *
     * @param multiplier - The fast forward multiplier to apply.
     */
    setFastForwardMultiplier(multiplier: number): void;

    /**
     * Sets the main loop timing mode and value.
     *
     * See: https://emscripten.org/docs/api_reference/emscripten.h.html#c.emscripten_set_main_loop_timing
     *
     * @param mode - Timing mode identifier.
     * @param value - Timing value associated with the mode.
     */
    setMainLoopTiming(mode: number, value: number): void;

    /**
     * Sets the audio volume multiplier.
     *
     * - `1.0` = 100%
     * - Range is `0.0..2.0` (0%..200%)
     *
     * @param percent - Volume multiplier to apply.
     */
    setVolume(percent: number): void;

    /**
     * Enables or disables keyboard input handling.
     *
     * @param enabled - If true input is accepted, if false input is ignored.
     */
    toggleInput(enabled: boolean): void;

    /**
     * Uploads a cheats file into the emulator filesystem.
     *
     * @param file - Cheats file to upload.
     * @param callback - Optional callback fired after upload completes.
     */
    uploadCheats(file: File, callback?: () => void): void;

    /**
     * Uploads a patch file into the emulator filesystem.
     *
     * @param file - Patch file to upload.
     * @param callback - Optional callback fired after upload completes.
     */
    uploadPatch(file: File, callback?: () => void): void;

    /**
     * Uploads a ROM file into the emulator filesystem.
     *
     * @param file - ROM file to upload.
     * @param callback - Optional callback fired after upload completes.
     */
    uploadRom(file: File, callback?: () => void): void;

    /**
     * Uploads a save or save state file into the emulator filesystem.
     *
     * @param file - Save or state file to upload.
     * @param callback - Optional callback fired after upload completes.
     */
    uploadSaveOrSaveState(file: File, callback?: () => void): void;

    /**
     * Uploads a screenshot file into the emulator filesystem.
     *
     * @param file - Screenshot file to upload.
     * @param callback - Optional callback fired after upload completes.
     */
    uploadScreenshot(file: File, callback?: () => void): void;

    /**
     * Registers core callbacks for emulator lifecycle and custom events.
     *
     * @param coreCallbacks - Callback object to register.
     */
    addCoreCallbacks(coreCallbacks: coreCallbacks): void;

    /**
     * Enables/disables active rewinding while the core is running.
     *
     * @param enabled - If true the core rewinds, if false rewind stops.
     */
    toggleRewind(enabled: boolean): void;

    /**
     * Applies core settings to the emulator runtime.
     *
     * @param coreSettings - Settings object to apply.
     */
    setCoreSettings(coreSettings: coreSettings): void;

    // custom variables
    version: {
      projectName: string;
      projectVersion: string;
    };

    /**
     * Returns common filesystem path strings used by this build.
     */
    filePaths(): filePaths;

    /**
     * Last loaded ROM path set after `loadGame` succeeds.
     */
    gameName?: string;

    /**
     * Save path used for the current game set after `loadGame` succeeds.
     */
    saveName?: string;

    /**
     * Auto save state path and file name used for the current game set after `loadGame` succeeds.
     */
    autoSaveStateName?: string;
    // extra exported runtime methods
    FS: typeof FS;
    // SDL2
    SDL2: {
      audio: {
        currentOutputBuffer: AudioBuffer;
        scriptProcessorNode: ScriptProcessorNode;
      };
      audioContext: AudioContext;
    };
  }

  /**
   * Generates a web assembly mGBA module for the browser.
   *
   * This emulator core is meant to be used like a controlled frontend component:
   * it bootstraps the emulator runtime, canvas wiring, keyboard events, and other autonomous
   * functions using emscripten, exposing an imperative API used to control the core.
   *
   * The consuming UI then drives all actions after initialization using the API above, such as
   * loading ROM/save files, pausing/resuming, settings, saves, etc.
   *
   * **Initialization flow:**
   * - create a `<canvas />`
   * - call `mGBA({ canvas })` to get the `Module`
   * - call `await Module.FSInit()` once to mount + load persisted file system data
   * - keep the `Module` accessible and call methods on it from your UI
   *
   * This core uses threads, your app must be served with cross-origin isolation enabled
   * (`COOP: same-origin` + `COEP: require-corp`), otherwise it will not run correctly.
   *
   * @param options - Module options.
   * @param options.canvas - Canvas used for video output.
   * @returns The initialized emulator `Module`.
   */
  // eslint-disable-next-line import/no-default-export
  export default function mGBA(options: {
    canvas: HTMLCanvasElement;
  }): Promise<mGBAEmulator>;
}

export = mGBA;
