#include "plugin.hpp"

// UI Layout constants
struct BlankLayout {
    static constexpr float startY = 30.0f;          // Starting Y position for controls
    static constexpr float rowSpacing = 60.0f;      // Vertical spacing between rows
    static constexpr float switchOffset = 30.0f;     // Vertical offset for switches below ports
    static constexpr float textOffset = 20.0f;       // Vertical offset for text below switches
    static constexpr float portTextOffset = 45.0f;   // Vertical offset for text below ports
    static constexpr float knobXOffset = 0.0f;       // X offset from center for knobs
    static constexpr float leftSwitchXOffset = -30.0f; // X offset from center for left switches
    static constexpr float rightSwitchXOffset = 30.0f; // X offset from center for right switches
    static constexpr float inputXOffset = -55.0f;    // X offset from center for input ports
    static constexpr float outputXOffset = 55.0f;    // X offset from center for output ports
    static constexpr float textYOffset = -3.0f;      // Fine adjustment for text Y positioning
};

struct BlankModule : Module
{
    enum InputIds {
        INPUT_1,
        INPUT_2,
        INPUT_3,
        INPUT_4,
        INPUT_5,
        INPUT_6,
        NUM_INPUTS
    };

    enum Params {
        ATTENUATION_KNOB1_PARAM,
        ATTENUATION_KNOB2_PARAM,
        ATTENUATION_KNOB3_PARAM,
        ATTENUATION_KNOB4_PARAM,
        ATTENUATION_KNOB5_PARAM,
        ATTENUATION_KNOB6_PARAM,
        SWITCH1_RANGE_PARAM,
        SWITCH2_RANGE_PARAM,
        SWITCH3_RANGE_PARAM,
        SWITCH4_RANGE_PARAM,
        SWITCH5_RANGE_PARAM,
        SWITCH6_RANGE_PARAM,
        SWITCH1_BIPOLAR_PARAM,
        SWITCH2_BIPOLAR_PARAM,
        SWITCH3_BIPOLAR_PARAM,
        SWITCH4_BIPOLAR_PARAM,
        SWITCH5_BIPOLAR_PARAM,
        SWITCH6_BIPOLAR_PARAM,
        SWITCH1_MODE_PARAM,
        SWITCH2_MODE_PARAM,
        SWITCH3_MODE_PARAM,
        SWITCH4_MODE_PARAM,
        SWITCH5_MODE_PARAM,
        SWITCH6_MODE_PARAM,
        NUM_PARAMS // last one counts how many
    };
    enum Outputs {
        OUTPUT_1,
        OUTPUT_2,
        OUTPUT_3,
        OUTPUT_4,
        OUTPUT_5,
        OUTPUT_6,
        NUM_OUTPUTS
    };
    enum Lights {
        // your lights
        NUM_LIGHTS
    };

    BlankModule()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (int i = 0; i < NUM_INPUTS; i++) {
            configInput(i, "Input " + std::to_string(i + 1));
        }

        for (int i = 0; i < 6; i++) {
            configParam(ATTENUATION_KNOB1_PARAM + i, 0.f, 1.f, 0.5f, "Knob " + std::to_string(i + 1));
            configParam(SWITCH1_BIPOLAR_PARAM + i, 0.f, 1.f, 0.f, "Bipolar Switch " + std::to_string(i + 1));
            configParam(SWITCH1_RANGE_PARAM + i, 0.f, 1.f, 0.f, "Range Switch " + std::to_string(i + 1));
            configParam(SWITCH1_MODE_PARAM + i, 0.f, 1.f, 0.f, "Mode Switch " + std::to_string(i + 1));
        }

        for (int i = 0; i < NUM_OUTPUTS; i++) {
            configOutput(i, "Output " + std::to_string(i + 1));
        }
    }

    // To save precious CPU, many modules will process parameter changes at 
    // a slower rate than the audio rate.
    //
    // This example shows a simple parameter update rate of 1/64th the sample rate.
    // You can use a ::rack::dsp::timer to update relative to clock time, rather
    // than change based on sample rate.
    //
    const int PARAM_INTERVAL = 64;
    int check_params = 0;
    void processParams()
    {
        // process parameters here
    }

    // To process inputs and outputs, override process()
    //
    void process(const ProcessArgs& args) override
    {
        // Process params at intervals
        if (++check_params > PARAM_INTERVAL) {
            check_params = 0;
            processParams();
        }

        // Process each output
        for (int i = 0; i < NUM_OUTPUTS; i++) {
            bool bipolar = params[SWITCH1_BIPOLAR_PARAM + i].getValue() == 0.f;
            bool shouldOffset = params[SWITCH1_MODE_PARAM + i].getValue() > 0.f;
            float knobValue = params[ATTENUATION_KNOB1_PARAM + i].getValue();
            float range = params[SWITCH1_RANGE_PARAM + i].getValue() == 0.f ? 5.f : 10.f;
            float signal = range;

            if (inputs[i].isConnected()) {
                signal = inputs[i].getVoltage();
            }

            if (shouldOffset) {
                signal += bipolar ? (knobValue * 2.f - 1.f) * range : knobValue * range;
            }
            else {
                signal *= bipolar ? (knobValue * 2.f - 1.f) : knobValue;
            }

            outputs[i].setVoltage(signal);
        }
    }

};

struct BlankModuleWidget : ModuleWidget
{
    BlankModuleWidget(BlankModule* module)
    {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Blank.svg"), asset::plugin(pluginInstance, "res/Blank-dark.svg")));

        // Add standard rack screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Add knobs in a column
        float knobX = box.size.x / 2; // Center of the module in pixels

        for (int i = 0; i < 6; i++) {
            addParam(createParamCentered<RoundLargeBlackKnob>(
                Vec(knobX + BlankLayout::knobXOffset,
                    BlankLayout::startY + i * BlankLayout::rowSpacing),
                module, BlankModule::ATTENUATION_KNOB1_PARAM + i));
        }

        // Add switches in a column between knobs and inputs
        for (int i = 0; i < 6; i++) {
            addParam(createParamCentered<CKSS>(
                Vec(knobX + BlankLayout::leftSwitchXOffset,
                    BlankLayout::startY + i * BlankLayout::rowSpacing),
                module, BlankModule::SWITCH1_RANGE_PARAM + i));
        }

        // Add switches in a column between knobs and outputs
        for (int i = 0; i < 6; i++) {
            addParam(createParamCentered<CKSS>(
                Vec(knobX + BlankLayout::rightSwitchXOffset,
                    BlankLayout::startY + i * BlankLayout::rowSpacing),
                module, BlankModule::SWITCH1_BIPOLAR_PARAM + i));
        }

        // Add input ports
        for (int i = 0; i < BlankModule::NUM_INPUTS; i++) {
            addInput(createInputCentered<PJ301MPort>(
                Vec(knobX + BlankLayout::inputXOffset,
                    BlankLayout::startY + i * BlankLayout::rowSpacing),
                module, BlankModule::INPUT_1 + i));
        }

        // Add output ports
        for (int i = 0; i < BlankModule::NUM_OUTPUTS; i++) {
            addOutput(createOutputCentered<PJ301MPort>(
                Vec(knobX + BlankLayout::outputXOffset,
                    BlankLayout::startY + i * BlankLayout::rowSpacing),
                module, BlankModule::OUTPUT_1 + i));
        }

        // Add switches underneath output ports
        for (int i = 0; i < 6; i++) {
            addParam(createParamCentered<CKSS>(
                Vec(knobX + BlankLayout::outputXOffset,
                    BlankLayout::startY + i * BlankLayout::rowSpacing + BlankLayout::switchOffset),
                module, BlankModule::SWITCH1_MODE_PARAM + i));
        }
    }

    void draw(const DrawArgs& args) override {
        ModuleWidget::draw(args);

        if (!module)
            return;

        float knobX = box.size.x / 2;

        // Draw text once for each row
        for (int i = 0; i < 6; i++) {
            nvgFillColor(args.vg, nvgRGB(0, 0, 0));
            nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            nvgFontSize(args.vg, 10);

            // Text above switches
            nvgText(args.vg,
                knobX + BlankLayout::leftSwitchXOffset,
                BlankLayout::startY + i * BlankLayout::rowSpacing - BlankLayout::textOffset + BlankLayout::textYOffset,
                "10V", NULL);

            nvgText(args.vg,
                knobX + BlankLayout::rightSwitchXOffset,
                BlankLayout::startY + i * BlankLayout::rowSpacing - BlankLayout::textOffset + BlankLayout::textYOffset,
                "UNI", NULL);
            // Text below switches
            nvgText(args.vg,
                knobX + BlankLayout::leftSwitchXOffset,
                BlankLayout::startY + i * BlankLayout::rowSpacing + BlankLayout::textOffset + BlankLayout::textYOffset,
                "5V", NULL);

            // Text for right switch
            nvgText(args.vg,
                knobX + BlankLayout::rightSwitchXOffset,
                BlankLayout::startY + i * BlankLayout::rowSpacing + BlankLayout::textOffset + BlankLayout::textYOffset,
                "BI", NULL);
        }

        // Add "NORMAL" text at the bottom of the module
        nvgFillColor(args.vg, nvgRGB(0, 0, 0));
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
        nvgFontSize(args.vg, 12);
        nvgText(args.vg, box.size.x / 2, box.size.y - 5, "NORMAL", NULL);
    }

    // Add options to your module's menu here
    //void appendContextMenu(Menu *menu) override
    //{
    //}
};

Model* modelBlank = createModel<BlankModule, BlankModuleWidget>("blank");
