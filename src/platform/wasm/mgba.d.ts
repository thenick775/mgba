/// <reference types="emscripten" />

declare namespace mGBA {
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
     * Typical values: 0..10
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
     * Typical values: 100..10000 is reasonable depending on memory pressure and
     * the rewind interval.
     *
     * Default: 600
     */
    rewindBufferCapacity?: number;

    /**
     * The speed at which rewind snapshots are taken. Larger numbers mean rewind happens faster.
     *
     * Typical values: 1..10
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
     * Typical values: 256..4096
     *
     * Default: 1024
     */
    audioBufferSize?: number;

    /**
     * Interval, in seconds, between periodic autosave-state captures. A value
     * of 0 disables the timer-based autosave.
     *
     * Typical values: 10..300
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
     * Can provide speedup on multi-core systems but is platform dependent.
     *
     * Default: false
     */
    threadedVideo?: boolean;

    /**
     * Enable/disable rewind. When true, rewind is available, when false rewind is disabled.
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
     * Show an on-screen (real) FPS counter overlay when set to true.
     *
     * Default: false
     */
    showFpsCounter?: boolean;

    /**
     * If true, attempt to automatically restore the most recent autosave
     * state when a game is loaded. If false, autosave states are ignored on
     * load and must be applied manually.
     *
     * Default: true
     */
    autoSaveStateEnable?: boolean;

    /**
     * If true, attempt to automatically restore the most recent autosave
     * state when a game is loaded. If false, autosave states are ignored on
     * load and must be applied manually.
     *
     * Default: true
     */
    restoreAutoSaveStateOnLoad?: boolean;
  };

  export interface mGBAEmulator extends EmscriptenModule {
    // custom methods from preamble
    autoLoadCheats(): boolean;
    bindKey(bindingName: string, inputName: string): void;
    buttonPress(name: string): void;
    buttonUnpress(name: string): void;
    FSInit(): Promise<void>;
    FSSync(): Promise<void>;
    getFastForwardMultiplier(): number;
    getMainLoopTimingMode(): number;
    getMainLoopTimingValue(): number;
    getSave(): Uint8Array | null;
    getVolume(): number;
    listRoms(): string[];
    listSaves(): string[];
    loadGame(romPath: string, savePathOverride?: string): boolean;
    loadState(slot: number): boolean;
    forceAutoSaveState(): boolean;
    loadAutoSaveState(): boolean;
    getAutoSaveState(): { autoSaveStateName: string; data: Uint8Array } | null;
    uploadAutoSaveState(
      autoSaveStateName: string,
      data: Uint8Array
    ): Promise<void>;
    pauseAudio(): void;
    pauseGame(): void;
    quickReload(): void;
    quitGame(): void;
    quitMgba(): void;
    resumeAudio(): void;
    resumeGame(): void;
    saveState(slot: number): boolean;
    screenshot(fileName?: string): boolean;
    setFastForwardMultiplier(multiplier: number): void;
    setMainLoopTiming(mode: number, value: number): void;
    setVolume(percent: number): void;
    toggleInput(enabled: boolean): void;
    uploadCheats(file: File, callback?: () => void): void;
    uploadPatch(file: File, callback?: () => void): void;
    uploadRom(file: File, callback?: () => void): void;
    uploadSaveOrSaveState(file: File, callback?: () => void): void;
    uploadScreenshot(file: File, callback?: () => void): void;
    addCoreCallbacks(coreCallbacks: coreCallbacks): void;
    toggleRewind(enabled: boolean): void;
    setCoreSettings(coreSettings: coreSettings): void;
    // custom variables
    version: {
      projectName: string;
      projectVersion: string;
    };
    filePaths(): filePaths;
    gameName?: string;
    saveName?: string;
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

  // eslint-disable-next-line import/no-default-export
  export default function mGBA(options: {
    canvas: HTMLCanvasElement;
  }): Promise<mGBAEmulator>;
}

export = mGBA;
