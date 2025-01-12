// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ConvexCollision/ConvexHullTest.h>
#include <Jolt/Geometry/ConvexHullBuilder.h>
#include <Utils/Log.h>
#include <Utils/AssetStream.h>
#include <Utils/DebugRendererSP.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <fstream>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_IMPLEMENT_RTTI_VIRTUAL(ConvexHullTest)
{
	JPH_ADD_BASE_CLASS(ConvexHullTest, Test)
}

void ConvexHullTest::Initialize()
{
	// First add a list of shapes that were problematic before
	mPoints = {
		{
			Vec3(-1, 0, -1),
			Vec3(1, 0, -1),
			Vec3(-1, 0, 1),
			Vec3(1, 0, 1)
		},
		{
			Vec3(-1, 0, -1),
			Vec3(1, 0, -1),
			Vec3(-1, 0, 1),
			Vec3(-0.5f, 0, -0.5f)
		},
		{
			Vec3(-1, 0, -1),
			Vec3(1, 0, -1),
			Vec3(-1, 0, 1),
			Vec3(1, 0, 1),
			Vec3(0, 1, 0)
		},
		{
			Vec3(1.25793016f, 0.157113776f, 1.22066617f),
			Vec3(1.92657053f, 0.157114446f, 0.240761176f),
			Vec3(1.40259242f, 0.157115221f, -0.834863901f),
			Vec3(1.94086421f, 0.157113507f, -0.790734947f),
			Vec3(2.20533752f, 0.157113209f, -0.281754375f),
			Vec3(0.0426187329f, 0.157113969f, -1.40533638f),
			Vec3(1.11055744f, 0.157113969f, -1.33626819f),
			Vec3(0.180490851f, 0.157114655f, 1.16420007f),
			Vec3(-1.34696794f, 0.157110974f, -0.978962243f),
			Vec3(-0.981223822f, 0.157110706f, -1.44589376f),
			Vec3(-1.8200444f, 0.157106474f, 1.05036092f),
			Vec3(-0.376947045f, 0.15711388f, 1.13544536f),
			Vec3(-1.37966835f, 0.157109678f, 1.08289516f),
			Vec3(-1.04599845f, 0.157108605f, 1.54891157f),
			Vec3(-0.597127378f, 0.157110557f, 1.57243586f),
			Vec3(-2.09407234f, 0.157106325f, 0.560136259f),
			Vec3(-1.91857386f, 0.157108605f, 0.0392456949f),
			Vec3(-2.08503342f, 0.157106936f, -0.506603181f),
			Vec3(-1.80278254f, 0.157107696f, -0.986931145f),
			Vec3(0.434835076f, 0.157112151f, 1.62568307f),
			Vec3(0.917346299f, 0.157111734f, 1.65097046f),
			Vec3(1.77710009f, 0.157112047f, 1.2388792f),
			Vec3(2.11432409f, 0.157112464f, 0.780689001f),
		},
		{
			Vec3(1.32055235f, -0.0982032791f, 0.020047307f),
			Vec3(-0.0175848603f, -0.104957283f, 0.020047307f),
			Vec3(-0.0175848603f, 0.098285675f, 0.020047307f),
			Vec3(1.32055235f, 0.098285675f, 0.020047307f),
			Vec3(1.00427914f, -0.0982032791f, 0.868395209f),
			Vec3(1.32055235f, -0.0982032791f, 2.63605499f),
			Vec3(1.00427914f, -0.0982032791f, 1.95698023f),
			Vec3(1.00427914f, -0.104957283f, 0.511006474f),
			Vec3(0.00150847435f, -0.104957283f, 0.511006474f),
			Vec3(0.271511227f, -0.179470509f, 0.868395209f),
			Vec3(0.00150847435f, -0.179470509f, 0.868395209f),
			Vec3(0.00150847435f, -0.179470509f, 0.511006474f),
			Vec3(0.271511227f, -0.179470509f, 0.511006474f),
			Vec3(1.00427914f, -0.145700991f, 1.95698023f),
			Vec3(1.00427914f, -0.145700991f, 2.40789247f),
			Vec3(0.271511227f, -0.179470509f, 2.40789247f),
			Vec3(0.271511227f, -0.179470509f, 1.95698023f),
			Vec3(0.00150847435f, -0.104957283f, 2.40789247f),
			Vec3(1.00427914f, -0.104957283f, 2.40789247f),
			Vec3(-0.0175848603f, -0.104957283f, 2.63605499f),
			Vec3(1.32055235f, 0.098285675f, 2.63605499f),
			Vec3(-0.0175848603f, 0.098285675f, 2.63605499f),
			Vec3(-0.0175848603f, -0.0929760709f, 1.31891572f),
			Vec3(-0.0175848603f, 0.0915316716f, 1.31891572f),
			Vec3(1.00427914f, -0.145700991f, 0.868395209f),
			Vec3(1.00427914f, -0.145700991f, 0.511006474f),
			Vec3(0.00150847435f, -0.104957283f, 0.868395209f),
			Vec3(0.00150847435f, -0.104957283f, 1.95698023f),
			Vec3(0.00150847435f, -0.179470509f, 1.95698023f),
			Vec3(0.00150847435f, -0.179470509f, 2.40789247f),
			Vec3(-0.0175848603f, -0.100129686f, 0.959797204f),
			Vec3(0.0878298879f, 0.139223307f, 1.04704332f),
			Vec3(0.122709334f, -0.147821367f, 1.15395057f),
			Vec3(0.122709334f, 0.139223307f, 1.15395057f),
			Vec3(0.19671753f, -0.118080139f, 1.15425301f),
			Vec3(0.0986568928f, -0.147821367f, 1.22612f),
			Vec3(0.175069571f, -0.118080139f, 1.2711879f),
			Vec3(-0.0175848603f, -0.147821367f, 0.959797204f),
			Vec3(0.0767889619f, -0.118080139f, 0.947003484f),
			Vec3(0.0878298879f, -0.147821367f, 1.04704332f),
			Vec3(0.18563965f, -0.118080139f, 1.03236175f),
			Vec3(-0.0175848603f, 0.098285675f, 0.959797204f),
			Vec3(0.0986568928f, 0.139223307f, 1.22612f),
			Vec3(0.0897113085f, -0.104957283f, 1.32667887f),
			Vec3(-0.0175848603f, -0.147821367f, 1.31891572f),
			Vec3(0.0897113085f, -0.118080139f, 1.32667887f),
			Vec3(0.175069571f, -0.104957283f, 1.2711879f),
			Vec3(0.18563965f, -0.104957283f, 1.03236175f),
			Vec3(0.19671753f, -0.104957283f, 1.15425301f),
			Vec3(0.0767889619f, -0.104957283f, 0.947003484f),
			Vec3(1.00427914f, 0.098285675f, 0.868395209f),
			Vec3(1.00427914f, 0.098285675f, 1.95698023f),
			Vec3(1.00427914f, 0.098285675f, 0.511006474f),
			Vec3(0.00150847435f, 0.098285675f, 0.511006474f),
			Vec3(0.00150847435f, 0.17087248f, 0.511006474f),
			Vec3(0.00150847435f, 0.17087248f, 0.868395209f),
			Vec3(0.271511227f, 0.17087248f, 0.868395209f),
			Vec3(0.271511227f, 0.17087248f, 0.511006474f),
			Vec3(0.271511227f, 0.17087248f, 2.40789247f),
			Vec3(1.00427914f, 0.137102962f, 2.40789247f),
			Vec3(1.00427914f, 0.137102962f, 1.95698023f),
			Vec3(0.271511227f, 0.17087248f, 1.95698023f),
			Vec3(0.00150847435f, 0.098285675f, 2.40789247f),
			Vec3(1.00427914f, 0.098285675f, 2.40789247f),
			Vec3(1.00427914f, 0.137102962f, 0.868395209f),
			Vec3(1.00427914f, 0.137102962f, 0.511006474f),
			Vec3(0.00150847435f, 0.098285675f, 0.868395209f),
			Vec3(0.00150847435f, 0.098285675f, 1.95698023f),
			Vec3(0.00150847435f, 0.17087248f, 1.95698023f),
			Vec3(0.00150847435f, 0.17087248f, 2.40789247f),
			Vec3(0.19671753f, 0.109482117f, 1.15425301f),
			Vec3(0.175069571f, 0.109482117f, 1.2711879f),
			Vec3(-0.0175848603f, 0.139223307f, 0.959797204f),
			Vec3(0.0767889619f, 0.109482117f, 0.947003484f),
			Vec3(0.18563965f, 0.109482117f, 1.03236175f),
			Vec3(0.0897113085f, 0.098285675f, 1.32667887f),
			Vec3(-0.0175848603f, 0.139223307f, 1.31891572f),
			Vec3(0.0897113085f, 0.109482117f, 1.32667887f),
			Vec3(0.175069571f, 0.098285675f, 1.2711879f),
			Vec3(0.19671753f, 0.098285675f, 1.15425301f),
			Vec3(0.18563965f, 0.098285675f, 1.03236175f),
			Vec3(0.0767889619f, 0.098285675f, 0.947003484f)
		},
		{
			Vec3(0.0212580804f, 1.29376173f, 0.0102035152f),
			Vec3(0.0225791596f, 1.05854928f, 0.0887729526f),
			Vec3(0.0596007220f, 0.984267414f, 0.0408750288f),
			Vec3(0.0722020790f, 0.980246127f, -0.0416274220f),
			Vec3(-0.00376634207f, -0.718282819f, 0.00411359267f),
			Vec3(-0.00188124576f, -0.718283117f, 0.00229378697f),
			Vec3(-0.00162511703f, -0.718282461f, 0.00753012672f),
			Vec3(-0.00118427153f, 1.36079276f, 0.00107491738f),
			Vec3(-6.78644137e-05f, -0.718282998f, 0.00426622201f),
			Vec3(0.00102991192f, 1.29927433f, 0.0230795704f),
			Vec3(0.00699944887f, 1.05855191f, 0.0887731761f),
			Vec3(-0.00603519706f, 1.04913890f, -0.102404378f),
			Vec3(-0.0212373994f, 1.31092644f, 0.00530112581f),
			Vec3(-0.0542707182f, 1.07623804f, 0.0403260253f),
			Vec3(-0.0946691483f, 1.07357991f, -0.0185115524f),
			Vec3(-0.0946691483f, 1.07357991f, -0.0185115524f)
		},
		{
			Vec3(0.0283679180f, 0.0443800166f, -0.00569444988f),
			Vec3(0.0327114500f, -0.0221119970f, 0.0232404359f),
			Vec3(0.0374971032f, 0.0148781445f, -0.0245264377f),
			Vec3(0.0439460576f, 0.0126368264f, 0.0197663195f),
			Vec3(-0.0327170566f, 0.0423904508f, 0.0181609988f),
			Vec3(-0.0306955911f, 0.0311534479f, -0.0281516202f),
			Vec3(-0.0262422040f, 0.0248970203f, 0.0450032614f),
			Vec3(-0.0262093470f, 0.00906597450f, 0.0481815264f),
			Vec3(-0.0256845430f, -0.00607067533f, -0.0401362479f),
			Vec3(-0.0179684199f, 0.0266145933f, -0.0394567028f),
			Vec3(-0.00567848794f, -0.0313231349f, -0.0263656937f),
			Vec3(-0.00444967486f, -0.0383231938f, 0.0206601117f),
			Vec3(-0.00329093798f, 0.0464436933f, 0.0343827978f),
			Vec3(-0.00225042878f, 0.0550651476f, -0.00304153794f),
			Vec3(0.00310287252f, 0.00219658483f, 0.0542362332f),
			Vec3(0.00435558241f, 0.00644031307f, -0.0455060303f),
			Vec3(0.00495047215f, -0.0144955292f, 0.0482611060f),
			Vec3(0.00510909408f, 0.0300753452f, -0.0415933356f),
			Vec3(0.00619197031f, 0.0269140154f, 0.0500008501f),
			Vec3(0.0190936550f, -0.0106478147f, 0.0453430638f),
			Vec3(0.0202461667f, 0.00821140409f, 0.0500608832f),
			Vec3(0.0199985132f, 0.0353404805f, 0.0413853638f),
			Vec3(0.0267947838f, -0.0155944452f, -0.0300960485f),
			Vec3(0.0274163429f, 0.0318853259f, -0.0288569275f),
			Vec3(-0.0404368788f, -0.0213200711f, -0.00530833099f),
			Vec3(-0.0383560173f, -0.0111571737f, 0.0346816145f),
			Vec3(-0.0453024730f, 0.00178011740f, -0.0218658112f),
			Vec3(-0.0482929349f, 0.0101582557f, 0.0191618335f)
		},
		{
			Vec3(0.19555497f, 0.06892325f, 0.21078214f),
			Vec3(0.20527978f, -0.01703966f, -0.09207391f),
			Vec3(0.21142941f, 0.01785821f, -0.09836373f),
			Vec3(0.21466828f, 0.05084385f, -0.03549951f),
			Vec3(-0.20511348f, -0.07018351f, -0.31925454f),
			Vec3(-0.19310803f, -0.13756239f, -0.33457401f),
			Vec3(-0.20095457f, -0.09572067f, -0.11383702f),
			Vec3(-0.18695570f, -0.14865115f, -0.19356145f),
			Vec3(-0.18073241f, -0.08639215f, -0.35319963f),
			Vec3(-0.18014188f, -0.15241129f, -0.34185338f),
			Vec3(-0.18174356f, -0.15312561f, -0.19147469f),
			Vec3(-0.19579467f, 0.01310298f, -0.00632396f),
			Vec3(-0.16814114f, -0.05610058f, -0.34890732f),
			Vec3(-0.16448530f, -0.16787034f, -0.29141789f),
			Vec3(-0.17525161f, 0.01533679f, 0.08730947f),
			Vec3(-0.17286175f, 0.08774700f, -0.01591185f),
			Vec3(-0.17077128f, 0.01983560f, 0.10070839f),
			Vec3(-0.14615997f, -0.16541340f, -0.37489247f),
			Vec3(-0.14595763f, -0.16490393f, -0.37515628f),
			Vec3(-0.16272801f, 0.07975677f, 0.08464866f),
			Vec3(-0.13369306f, -0.06286648f, -0.37556374f),
			Vec3(-0.14785704f, 0.14323678f, -0.01563696f),
			Vec3(-0.12817731f, -0.04268694f, -0.36287897f),
			Vec3(-0.14112462f, 0.13547241f, 0.05140329f),
			Vec3(-0.12341158f, -0.17782864f, -0.36954373f),
			Vec3(-0.12310848f, -0.18070405f, -0.20412853f),
			Vec3(-0.09967888f, -0.18289816f, -0.38768309f),
			Vec3(-0.09960851f, 0.14144828f, 0.12903015f),
			Vec3(-0.08962545f, -0.17236463f, -0.39919903f),
			Vec3(-0.09338194f, -0.00865331f, 0.23358464f),
			Vec3(-0.09496998f, 0.17418922f, 0.03730623f),
			Vec3(-0.09499961f, 0.16077143f, -0.03914160f),
			Vec3(-0.08221246f, -0.07778487f, -0.39787262f),
			Vec3(-0.07918695f, -0.14616625f, -0.40242865f),
			Vec3(-0.08256439f, 0.01469633f, 0.24209134f),
			Vec3(-0.07199146f, 0.16959090f, 0.11185526f),
			Vec3(-0.05876892f, -0.18819671f, -0.40239989f),
			Vec3(-0.05744339f, -0.18692162f, -0.40386000f),
			Vec3(-0.04441069f, -0.04126521f, -0.37501192f),
			Vec3(-0.04648328f, 0.18093951f, 0.03905040f),
			Vec3(-0.03611449f, -0.14904837f, -0.40508240f),
			Vec3(-0.03163360f, 0.17144355f, 0.13303288f),
			Vec3(-0.02255749f, -0.01798030f, 0.33883106f),
			Vec3(-0.01062212f, -0.11764656f, -0.39784804f),
			Vec3(0.00002799f, -0.18946082f, -0.39155373f),
			Vec3(0.00190875f, -0.16691279f, -0.40337407f),
			Vec3(0.02337403f, -0.03170533f, 0.38295418f),
			Vec3(0.02689898f, -0.03111388f, 0.38642361f),
			Vec3(0.03513940f, -0.09795553f, -0.38733068f),
			Vec3(0.04139633f, -0.18845227f, -0.32015734f),
			Vec3(0.04843888f, 0.12765829f, -0.09677977f),
			Vec3(0.04454701f, -0.14539991f, -0.38590988f),
			Vec3(0.04690936f, -0.17584648f, -0.38177087f),
			Vec3(0.05052238f, -0.18907529f, -0.35411724f),
			Vec3(0.07129140f, -0.02806735f, 0.41684112f),
			Vec3(0.07599759f, 0.02516599f, 0.43382310f),
			Vec3(0.08328492f, -0.18135514f, -0.32588836f),
			Vec3(0.08443428f, 0.07232403f, 0.37877142f),
			Vec3(0.09074404f, -0.15272216f, -0.36002999f),
			Vec3(0.09381036f, -0.04931259f, -0.32999005f),
			Vec3(0.09348832f, -0.17767928f, -0.33666068f),
			Vec3(0.09247280f, -0.01328942f, 0.44227284f),
			Vec3(0.09364306f, 0.03557658f, 0.44191616f),
			Vec3(0.09611026f, -0.01203391f, 0.44345939f),
			Vec3(0.09662163f, 0.03456752f, 0.44326156f),
			Vec3(0.10482377f, 0.12817247f, 0.27224415f),
			Vec3(0.11271536f, 0.12685699f, 0.26856660f),
			Vec3(0.10957191f, 0.03837919f, 0.43455946f),
			Vec3(0.11146642f, -0.01284471f, 0.42120608f),
			Vec3(0.11088928f, 0.00377234f, 0.44789928f),
			Vec3(0.11571233f, -0.12474029f, -0.34762913f),
			Vec3(0.12183426f, -0.16410264f, -0.30295142f),
			Vec3(0.12211698f, 0.01099167f, 0.44373258f),
			Vec3(0.12308656f, 0.01315179f, 0.44303578f),
			Vec3(0.13090495f, -0.15086941f, -0.31031519f),
			Vec3(0.14427974f, 0.09778974f, 0.30786031f),
			Vec3(0.14200252f, 0.01419945f, 0.41783332f),
			Vec3(0.14424091f, 0.06972501f, 0.37377491f),
			Vec3(0.14422383f, 0.02227210f, 0.41717034f),
			Vec3(0.15133176f, -0.03861540f, -0.27380293f),
			Vec3(0.14738929f, 0.06972805f, 0.37101438f),
			Vec3(0.15116664f, -0.13012324f, -0.26891800f),
			Vec3(0.15432675f, -0.05065062f, -0.27696538f),
			Vec3(0.17231981f, 0.09891064f, -0.04109610f),
			Vec3(0.15486444f, 0.03080789f, 0.39333733f),
			Vec3(0.16293872f, 0.09977609f, 0.23133035f),
			Vec3(0.17278114f, 0.05925680f, -0.13166353f),
			Vec3(0.17344120f, 0.06815492f, 0.29800513f),
			Vec3(0.18346339f, 0.03002923f, -0.16944433f),
			Vec3(0.18475264f, -0.03337195f, -0.21144425f),
			Vec3(0.18153211f, 0.05077920f, 0.29410797f),
			Vec3(0.18872119f, 0.08419117f, 0.18681980f),
			Vec3(0.19402013f, 0.03129275f, -0.14645814f),
			Vec3(0.20299899f, 0.06450803f, -0.05323168f),
			Vec3(-0.20916573f, -0.14482390f, -0.28754678f),
			Vec3(-0.21912349f, -0.12297497f, -0.25853595f),
			Vec3(-0.21891747f, -0.11492035f, -0.30946639f),
			Vec3(-0.22503024f, -0.09871494f, -0.27031892f),
			Vec3(-0.22503024f, -0.09871494f, -0.27031892f),
			Vec3(-0.22503024f, -0.09871494f, -0.27031892f)
		},
		{
			Vec3(0.28483882f, 0.09470236f, 0.11433057f),
			Vec3(0.30260321f, 0.07340867f, 0.00849266f),
			Vec3(0.30380272f, 0.05582517f, -0.22405298f),
			Vec3(0.30670973f, 0.02778204f, -0.22415190f),
			Vec3(-0.29766368f, -0.06492511f, -0.19135096f),
			Vec3(-0.28324991f, 0.02856347f, 0.16558051f),
			Vec3(-0.27339774f, 0.11253071f, -0.13812468f),
			Vec3(-0.26324614f, -0.03483995f, 0.34903234f),
			Vec3(-0.27118766f, -0.15035510f, -0.06431498f),
			Vec3(-0.26041472f, 0.10464326f, -0.20795805f),
			Vec3(-0.22156618f, -0.00712212f, 0.40348106f),
			Vec3(-0.20013636f, 0.13795423f, -0.23888915f),
			Vec3(-0.19368620f, 0.04208890f, 0.42129427f),
			Vec3(-0.18170905f, -0.10169907f, 0.38139578f),
			Vec3(-0.18724660f, 0.18995818f, 0.08522552f),
			Vec3(-0.17479378f, -0.05597380f, 0.41057986f),
			Vec3(-0.15012621f, 0.08595391f, 0.43914794f),
			Vec3(-0.11722116f, -0.10298516f, -0.30289822f),
			Vec3(-0.11217459f, 0.00596011f, 0.44133874f),
			Vec3(-0.11709289f, 0.23012112f, 0.12055066f),
			Vec3(-0.10705470f, 0.15775623f, -0.33419770f),
			Vec3(-0.08655276f, 0.09824081f, 0.43651989f),
			Vec3(-0.08401379f, 0.08668444f, -0.41111666f),
			Vec3(-0.08026488f, -0.24695427f, -0.01228247f),
			Vec3(-0.06294082f, 0.12666735f, -0.39178270f),
			Vec3(-0.05308891f, -0.07724215f, -0.37346649f),
			Vec3(-0.04869145f, -0.23846265f, -0.11154356f),
			Vec3(-0.04377052f, 0.06346821f, 0.44263243f),
			Vec3(-0.03821557f, 0.05776290f, -0.43330976f),
			Vec3(-0.01401243f, -0.07849873f, 0.37016886f),
			Vec3(-0.01267736f, -0.24327334f, -0.09846258f),
			Vec3(-0.00871999f, -0.24532425f, -0.01158716f),
			Vec3(0.00610917f, 0.20575316f, -0.32363408f),
			Vec3(0.01893912f, -0.02637211f, -0.44099009f),
			Vec3(0.03742292f, 0.25572568f, 0.11976100f),
			Vec3(0.04572892f, -0.02452080f, 0.37599292f),
			Vec3(0.04809525f, 0.11413645f, 0.38247618f),
			Vec3(0.04934106f, -0.01875172f, -0.43612641f),
			Vec3(0.07854398f, 0.13351599f, 0.34539741f),
			Vec3(0.11064179f, 0.03347895f, 0.33272063f),
			Vec3(0.11110801f, 0.04016598f, -0.42360800f),
			Vec3(0.12390327f, -0.20230874f, -0.01599736f),
			Vec3(0.13082972f, -0.19843940f, -0.08606190f),
			Vec3(0.12559986f, -0.02563187f, -0.38013845f),
			Vec3(0.12924608f, 0.16206453f, -0.34893369f),
			Vec3(0.15646456f, 0.21451330f, 0.16623015f),
			Vec3(0.17851203f, -0.14074428f, 0.08427754f),
			Vec3(0.19401437f, -0.15288332f, -0.03272480f),
			Vec3(0.20102191f, 0.08705597f, -0.37915167f),
			Vec3(0.20596674f, 0.06604006f, -0.38868805f),
			Vec3(0.26085311f, 0.08702713f, -0.32507085f),
			Vec3(0.27331018f, 0.15497627f, 0.11259682f),
			Vec3(0.27269470f, 0.03719006f, -0.31962081f),
			Vec3(0.27288356f, 0.06217747f, -0.33064606f),
			Vec3(-0.29314118f, -0.18079891f, 0.24351751f),
			Vec3(-0.30831277f, -0.06952596f, 0.07340523f),
			Vec3(-0.30126276f, -0.18365636f, 0.22815129f),
			Vec3(-0.30392047f, -0.17969127f, 0.22713920f),
			Vec3(-0.30392047f, -0.17969127f, 0.22713920f),
			Vec3(-0.30392047f, -0.17969127f, 0.22713920f)
		},
		{
			// A really small hull
			Vec3(-0.00707678869f, 0.00559568405f, -0.0239779726f),
			Vec3(0.0136205591f, 0.00541752577f, -0.0225500446f),
			Vec3(0.0135576781f, 0.00559568405f, -0.0224227905f),
			Vec3(-0.0108219199f, 0.00559568405f, -0.0223935191f),
			Vec3(0.0137226451f, 0.00559568405f, -0.0220940933f),
			Vec3(0.00301175844f, -0.0232942104f, -0.0214947499f),
			Vec3(0.017349612f, 0.00559568405f, 0.0241708681f),
			Vec3(0.00390899926f, -0.0368074179f, 0.0541367307f),
			Vec3(-0.0164459459f, 0.00559568405f, 0.0607497096f),
			Vec3(-0.0169881769f, 0.00559568405f, 0.0608173609f),
			Vec3(-0.0168782212f, 0.0052883029f, 0.0613293499f),
			Vec3(-0.00663783913f, 0.00559568405f, -0.024154868f),
			Vec3(-0.00507298298f, 0.00559568405f, -0.0242112875f),
			Vec3(-0.00565947127f, 0.00477081537f, -0.0243848339f),
			Vec3(0.0118075963f, 0.00124305487f, -0.0258472487f),
			Vec3(0.00860248506f, -0.00697988272f, -0.0276725553f),
		},
		{
			// Nearly co-planar hull (but not enough to go through the 2d hull builder)
			Vec3(0.129325435f, -0.213319957f, 0.00901593268f),
			Vec3(0.129251331f, -0.213436425f, 0.00932094082f),
			Vec3(0.160741553f, -0.171540618f, 0.0494558439f),
			Vec3(0.160671368f, -0.17165187f, 0.049765937f),
			Vec3(0.14228563f, 0.432965666f, 0.282429159f),
			Vec3(0.142746598f, 0.433226734f, 0.283286631f),
			Vec3(0.296031296f, 0.226935148f, 0.312804461f),
			Vec3(0.296214104f, 0.227568939f, 0.313606918f),
			Vec3(-0.00354258716f, -0.180767179f, -0.0762089267f),
			Vec3(-0.00372517109f, -0.1805875f, -0.0766792595f),
			Vec3(-0.0157070309f, -0.176182508f, -0.0833940506f),
			Vec3(-0.0161666721f, -0.175898403f, -0.0840280354f),
			Vec3(-0.342764735f, 0.0259497911f, -0.244388372f),
			Vec3(-0.342298329f, 0.0256615728f, -0.24456653f),
			Vec3(-0.366584063f, 0.0554589033f, -0.250078142f),
			Vec3(-0.366478682f, 0.0556178838f, -0.250342518f),
		},
		{
			// A hull with a very acute angle that won't properly build when using distance to plane only
			Vec3(-0.0451235026f, -0.103826642f, -0.0346511155f),
			Vec3(-0.0194563419f, -0.123563275f, -0.032212317f),
			Vec3(0.0323024541f, -0.0468643308f, -0.0307639092f),
			Vec3(0.0412166864f, -0.0884782523f, -0.0288816988f),
			Vec3(-0.0564572513f, 0.0207469314f, 0.0169318169f),
			Vec3(0.00537410378f, 0.105688639f, 0.0355164111f),
			Vec3(0.0209896415f, 0.117749952f, 0.0365252197f),
			Vec3(0.0211542398f, 0.118546993f, 0.0375355929f),
		}
	};

	// Add a cube formed out of a regular grid of vertices, this shows how the algorithm deals
	// with many coplanar points
	{
		Points p;
		for (int x = 0; x < 10; ++x)
			for (int y = 0; y < 10; ++y)
				for (int z = 0; z < 10; ++z)
					p.push_back(Vec3::sReplicate(-0.5f) * 0.1f * Vec3(float(x), float(y), float(z)));
		mPoints.push_back(std::move(p));
	}

	// Add disc of many points
	{
		Points p;
		Mat44 rot = Mat44::sRotationZ(0.25f * JPH_PI);
		for (float r = 0.0f; r < 2.0f; r += 0.1f)
			for (float phi = 0.0f; phi <= 2.0f * JPH_PI; phi += 2.0f * JPH_PI / 20.0f)
				p.push_back(rot * Vec3(r * Cos(phi), r * Sin(phi), 0));
		mPoints.push_back(std::move(p));
	}

	// Add wedge shaped disc that is just above the hull tolerance on its widest side and zero on the other side
	{
		Points p;
		for (float phi = 0.0f; phi <= 2.0f * JPH_PI; phi += 2.0f * JPH_PI / 40.0f)
		{
			Vec3 pos(2.0f * Cos(phi), 0, 2.0f * Sin(phi));
			p.push_back(pos);
			p.push_back(pos + Vec3(0, 2.0e-3f * (2.0f + pos.GetX()) / 4.0f, 0));
		}
		mPoints.push_back(std::move(p));
	}

	// Add a sphere of many points
	{
		Points p;
		for (float theta = 0.0f; theta <= JPH_PI; theta += JPH_PI / 20.0f)
			for (float phi = 0.0f; phi <= 2.0f * JPH_PI; phi += 2.0f * JPH_PI / 20.0f)
				p.push_back(Vec3::sUnitSpherical(theta, phi));
		mPoints.push_back(std::move(p));
	}

	// Open the external file with hulls
	// A stream containing predefined convex hulls
	AssetStream points_asset_stream("convex_hulls.bin", std::ios::in | std::ios::binary);
	std::istream &points_stream = points_asset_stream.Get();
	for (;;)
	{
		// Read the length of the next point cloud
		uint32 len = 0;
		points_stream.read((char *)&len, sizeof(len));
		if (points_stream.eof())
			break;

		// Read the points
		if (len > 0)
		{
			Points p;
			for (uint32 i = 0; i < len; ++i)
			{
				Float3 v;
				points_stream.read((char *)&v, sizeof(v));
				p.push_back(Vec3(v));
			}
			mPoints.push_back(std::move(p));
		}
	}
}

void ConvexHullTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	const float display_scale = 10.0f;

	float tolerance = 1.0e-3f;

	Points points;
	if (mIteration < mPoints.size())
	{
		// Take one of the predefined shapes
		points = mPoints[mIteration];
	}
	else
	{
		uniform_real_distribution<float> zero_one(0.0f, 1.0f);
		uniform_real_distribution<float> zero_two(0.0f, 2.0f);

		// Define vertex scale
		uniform_real_distribution<float> scale_start(0.1f, 0.5f);
		uniform_real_distribution<float> scale_range(0.1f, 2.0f);
		float start = scale_start(mRandom);
		uniform_real_distribution<float> vertex_scale(start, start + scale_range(mRandom));

		// Define shape scale to make shape less sphere like
		uniform_real_distribution<float> shape_scale(0.1f, 1.0f);
		Vec3 scale(shape_scale(mRandom), shape_scale(mRandom), shape_scale(mRandom));

		// Add some random points
		for (int i = 0; i < 100; ++i)
		{
			// Add random point
			Vec3 p1 = vertex_scale(mRandom) * Vec3::sRandom(mRandom) * scale;
			points.push_back(p1);

			// Point close to p1
			Vec3 p2 = p1 + tolerance * zero_two(mRandom) * Vec3::sRandom(mRandom);
			points.push_back(p2);

			// Point on a line to another point
			float fraction = zero_one(mRandom);
			Vec3 p3 = fraction * p1 + (1.0f - fraction) * points[mRandom() % points.size()];
			points.push_back(p3);

			// Point close to p3
			Vec3 p4 = p3 + tolerance * zero_two(mRandom) * Vec3::sRandom(mRandom);
			points.push_back(p4);
		}
	}
	mIteration++;

	using Face = ConvexHullBuilder::Face;
	using Edge = ConvexHullBuilder::Edge;
	ConvexHullBuilder builder(points);

	// Build the hull
	const char *error = nullptr;
	ConvexHullBuilder::EResult result = builder.Initialize(INT_MAX, tolerance, error);
	if (result != ConvexHullBuilder::EResult::Success && result != ConvexHullBuilder::EResult::MaxVerticesReached)
	{
		Trace("Iteration %d: Failed to initialize from positions: %s", mIteration - 1, error);
		JPH_ASSERT(false);
		return;
	}

	// Determine center of mass
	Vec3 com;
	float vol;
	builder.GetCenterOfMassAndVolume(com, vol);

	// Test if all points are inside the hull with the given tolerance
	float max_error, coplanar_distance;
	int max_error_point;
	Face *max_error_face;
	builder.DetermineMaxError(max_error_face, max_error, max_error_point, coplanar_distance);

	// Check if error is bigger than 4 * the tolerance
	if (max_error > 4.0f * max(tolerance, coplanar_distance))
	{
		Trace("Iteration %d: max_error=%g", mIteration - 1, (double)max_error);

		// Draw point that had the max error
		Vec3 point = display_scale * (points[max_error_point] - com);
		DrawMarkerSP(mDebugRenderer, point, Color::sRed, 1.0f);
		DrawText3DSP(mDebugRenderer, point, StringFormat("%d: %g", max_error_point, (double)max_error), Color::sRed);

		// Length of normal (2x area) for max error face
		Vec3 centroid = display_scale * (max_error_face->mCentroid - com);
		Vec3 centroid_plus_normal = centroid + max_error_face->mNormal.Normalized();
		DrawArrowSP(mDebugRenderer, centroid, centroid_plus_normal, Color::sGreen, 0.1f);
		DrawText3DSP(mDebugRenderer, centroid_plus_normal, ConvertToString(max_error_face->mNormal.Length()), Color::sGreen);

		// Draw face that had the max error
		const Edge *e = max_error_face->mFirstEdge;
		Vec3 prev = display_scale * (points[e->mStartIdx] - com);
		do
		{
			const Edge *next = e->mNextEdge;
			Vec3 cur = display_scale * (points[next->mStartIdx] - com);
			DrawArrowSP(mDebugRenderer, prev, cur, Color::sYellow, 0.01f);
			DrawText3DSP(mDebugRenderer, prev, ConvertToString(e->mStartIdx), Color::sYellow);
			e = next;
			prev = cur;
		} while (e != max_error_face->mFirstEdge);

		JPH_ASSERT(false);
	}

	// Draw input points around center of mass
	for (Vec3 v : points)
		DrawMarkerSP(mDebugRenderer, display_scale * (v - com), Color::sWhite, 0.01f);

	// Draw the hull around center of mass
	int color_idx = 0;
	for (Face *f : builder.GetFaces())
	{
		Color color = Color::sGetDistinctColor(color_idx++);

		// First point
		const Edge *e = f->mFirstEdge;
		Vec3 p1 = display_scale * (points[e->mStartIdx] - com);

		// Second point
		e = e->mNextEdge;
		Vec3 p2 = display_scale * (points[e->mStartIdx] - com);

		// First line
		DrawLineSP(mDebugRenderer, p1, p2, Color::sGrey);

		do
		{
			// Third point
			e = e->mNextEdge;
			Vec3 p3 = display_scale * (points[e->mStartIdx] - com);

			DrawTriangleSP(mDebugRenderer, p1, p2, p3, color);

			DrawLineSP(mDebugRenderer, p2, p3, Color::sGrey);

			p2 = p3;
		}
		while (e != f->mFirstEdge);
	}
}
